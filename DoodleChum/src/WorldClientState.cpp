/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

DoodleChum - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <WorldClientState.hpp>
#include <MeshIDs.hpp>
#include <DayNightCycle.hpp>
#include <RoomLightController.hpp>
#include <BudController.hpp>
#include <MessageIDs.hpp>

#include <xygine/App.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Reports.hpp>
#include <xygine/components/MeshDrawable.hpp>
#include <xygine/mesh/IQMBuilder.hpp>
#include <xygine/mesh/QuadBuilder.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/tilemap/Map.hpp>
#include <xygine/components/TileMapLayer.hpp>
#include <xygine/shaders/Tilemap.hpp>
#include <xygine/components/PointLight.hpp>
#include <xygine/tilemap/TileLayer.hpp>
#include <xygine/components/ParticleController.hpp>

#include <xygine/postprocess/ChromeAb.hpp>
#include <xygine/postprocess/Antique.hpp>

#include <SFML/Graphics/RectangleShape.hpp>

namespace
{
    sf::Vector2f spawnPosition;
    sf::Vector2f budSize(90.f, 180.f);
}

using namespace std::placeholders;

WorldClientState::WorldClientState(xy::StateStack& stateStack, Context context)
    : State                 (stateStack, context),
    m_messageBus            (context.appInstance.getMessageBus()),
    m_scene                 (m_messageBus),
    m_meshRenderer          ({ context.appInstance.getVideoSettings().VideoMode.width, context.appInstance.getVideoSettings().VideoMode.height }, m_scene)
{
    launchLoadingScreen();
    m_scene.setView(context.defaultView);

    //auto pp = xy::PostProcess::create<xy::PostChromeAb>();
    //auto pp = xy::PostProcess::create<xy::PostAntique>();
    //m_scene.addPostProcess(pp);

    initMeshes();
    initMapData();
    initBud();
    initParticles();
    initUI();

    quitLoadingScreen();
}

//public
bool WorldClientState::update(float dt)
{    
    m_scene.update(dt);
    m_meshRenderer.update();
    return false;
}

bool WorldClientState::handleEvent(const sf::Event& evt)
{
    return false;
}

void WorldClientState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
    m_meshRenderer.handleMessage(msg);

    if (msg.id == xy::Message::UIMessage)
    {
        const auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::ResizedWindow:
        {
            /*auto v = playerCamera->getView();
            v.setViewport(getContext().defaultView.getViewport());
            playerCamera->setView(v);*/
            m_scene.setView(getContext().defaultView);
            //m_meshRenderer.setView(getContext().defaultView);
        }
        break;
        }
    }
}

void WorldClientState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    //rw.draw(m_meshRenderer);
    rw.draw(m_pathFinder);
}

//private
void WorldClientState::initMeshes()
{
    m_meshRenderer.setView(getContext().defaultView);
    m_meshRenderer.setFOV(45.f);

    m_shaderResource.preload(Shader::TexturedBumped, DEFERRED_TEXTURED_BUMPED_VERTEX, DEFERRED_TEXTURED_BUMPED_FRAGMENT);
    m_shaderResource.preload(Shader::Shadow, SHADOW_VERTEX, SHADOW_FRAGMENT);
    
    xy::IQMBuilder haus("assets/models/haus.iqm");
    m_meshRenderer.loadModel(Mesh::Haus, haus);

    auto& hausMat = m_materialResource.add(Material::Haus, m_shaderResource.get(Shader::TexturedBumped));
    hausMat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    hausMat.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/textures/haus_diffuse.png") });
    hausMat.addProperty({ "u_normalMap", m_textureResource.get("assets/images/textures/haus_normal.png") });
    //hausMat.addProperty({ "u_maskMap", m_textureResource.get("assets/images/mask.png") });
    hausMat.addRenderPass(xy::RenderPass::ID::ShadowMap, m_shaderResource.get(Shader::Shadow));
    hausMat.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);

    auto hausModel = m_meshRenderer.createModel(Mesh::Haus, m_messageBus);
    hausModel->setBaseMaterial(hausMat);
    hausModel->setPosition({ 0.f, 0.f, 64.f });
    //hausModel->setScale({ 1.f, 1.f, -1.f });

    xy::IQMBuilder background("assets/models/background.iqm");
    m_meshRenderer.loadModel(Mesh::Background, background);

    auto& backMat = m_materialResource.add(Material::Background, m_shaderResource.get(Shader::TexturedBumped));
    backMat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    backMat.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/textures/background_diffuse.png") });
    backMat.addProperty({ "u_normalMap", m_textureResource.get("assets/images/textures/haus_normal.png") });

    auto backgroundModel = m_meshRenderer.createModel(Mesh::Background, m_messageBus);
    backgroundModel->setBaseMaterial(backMat);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(hausModel);
    entity->addComponent(backgroundModel);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    m_scene.addEntity(entity, xy::Scene::BackFront);

    entity = xy::Entity::create(m_messageBus);
    auto md = m_meshRenderer.createDrawable(m_messageBus);
    entity->addComponent(md);
    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);

    //quad for player
    xy::QuadBuilder qb(budSize);
    m_meshRenderer.loadModel(Mesh::Bud, qb);
    
    m_textureResource.setFallbackColour({ 127, 127, 255 });

    auto& budMat = m_materialResource.add(Material::Bud, m_shaderResource.get(Shader::TexturedBumped));
    budMat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    //budMat.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/textures/db_diffuse.png") });
    budMat.addProperty({ "u_normalMap", m_textureResource.get("fallback") });
    budMat.addRenderPass(xy::RenderPass::ID::ShadowMap, m_shaderResource.get(Shader::Shadow));
}

void WorldClientState::initMapData()
{
    xy::tmx::Map map;
    if (map.load("assets/mapdata/map.tmx"))
    {
        m_shaderResource.preload(xy::Shader::Tilemap, xy::Shader::tmx::vertex, xy::Shader::tmx::fragment);

        sf::Vector2f mapOffset = (xy::DefaultSceneSize / 2.f) - sf::Vector2f(map.getBounds().width / 2.f, map.getBounds().height / 2.f);
        m_pathFinder.setTileSize(static_cast<sf::Vector2f>(map.getTileSize()));
        m_pathFinder.setGridOffset(mapOffset);
        m_pathFinder.setGridSize(map.getTileCount());

        const auto& layers = map.getLayers();
        for (const auto& l : layers)
        {
            if (l->getType() == xy::tmx::Layer::Type::Tile)
            {
                //TODO check dwb is not nullptr
                /*auto dwb = map.getDrawable(m_messageBus, *l, m_textureResource, m_shaderResource);
                auto entity = xy::Entity::create(m_messageBus);
                entity->addComponent(dwb);
                entity->setPosition(mapOffset);
                m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);*/


                xy::tmx::TileLayer* tl = dynamic_cast<xy::tmx::TileLayer*>(l.get());
                const auto& tiles = tl->getTiles();
                auto mapSize = map.getTileCount();
                for (auto y = 0u; y < mapSize.y; ++y)
                {
                    for (auto x = 0u; x < mapSize.x; ++x)
                    {
                        if (tiles[y * mapSize.x + x].ID != 0)
                        {
                            const auto& name = l->getName();
                            if (name == "navigation")
                            {
                                //any tile counts as a wall
                                m_pathFinder.addSolidTile({ x, y });
                            }
                        }
                    }           
                }
            }
            else if (l->getType() == xy::tmx::Layer::Type::Object)
            {
                const auto& name = l->getName();
                if (name == "lights")
                {
                    const auto& objs = dynamic_cast<xy::tmx::ObjectGroup*>(l.get())->getObjects();
                    for (const auto& o : objs)
                    {
                        auto light = xy::Component::create<xy::PointLight>(m_messageBus, 640.f, 20.f, sf::Color(255,240,220));
                        light->setDepth(260.f);
                        light->setIntensity(1.f);
                        //light->enableShadowCasting(true);

                        auto controller = xy::Component::create<RoomLightController>(m_messageBus);

                        auto entity = xy::Entity::create(m_messageBus);
                        entity->setPosition(o.getPosition() + mapOffset);
                        entity->addComponent(light);
                        entity->addComponent(controller);
                        m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
                    }
                }
                else if (name == "tasks")
                {
                    const auto& objs = dynamic_cast<xy::tmx::ObjectGroup*>(l.get())->getObjects();
                    for (const auto& obj : objs)
                    {
                        m_tasks.emplace_back();
                        auto& task = m_tasks.back();
                        task.name = obj.getName();
                        task.position.x = static_cast<sf::Uint32>(obj.getPosition().x) / map.getTileSize().x;
                        task.position.y = static_cast<sf::Uint32>(obj.getPosition().y) / map.getTileSize().y;
                        
                        const auto& properties = obj.getProperties();
                        for (const auto& prop : properties)
                        {
                            if (prop.getType() == xy::tmx::Property::Type::Int)
                            {
                                const auto propName = prop.getName();
                                if (propName == "id")
                                {
                                    task.id = prop.getIntValue();
                                }
                            }
                        }
                    }
                }
            }
        }

        spawnPosition = xy::DefaultSceneSize / 2.f;
        spawnPosition.y += map.getBounds().height / 2.f;

        if (!m_pathFinder.hasData())
        {
            xy::Logger::log("No navigation data has been loaded!", xy::Logger::Type::Error);
        }
        //m_pathFinder.plotPath({ 26u, 29u }, { 49u, 9u });
    }
    else
    {
        xy::Logger::log("Failed to open map data", xy::Logger::Type::Error, xy::Logger::Output::All);
    }
}

void WorldClientState::initBud()
{
    auto controller = xy::Component::create<BudController>(m_messageBus, m_pathFinder, m_tasks, m_textureResource.get("assets/images/sprites/bud.png"));

    auto& material = m_materialResource.get(Material::Bud);
    material.addProperty({ "u_diffuseMap", controller->getTexture() });

    auto dwb = m_meshRenderer.createModel(Mesh::Bud, m_messageBus);
    dwb->setBaseMaterial(material);
    dwb->setPosition({ 0.f, -budSize.y / 2.f, 4.f });

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(dwb);
    entity->addComponent(controller);
    entity->setPosition(spawnPosition);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void WorldClientState::initParticles()
{
    xy::ParticleSystem::Definition steam;
    steam.loadFromFile("assets/particles/steam.xyp", m_textureResource);
    auto entity = xy::Entity::create(m_messageBus);
    entity->addCommandCategories(Particle::Steam);
    auto ps = steam.createSystem(m_messageBus);
    entity->addComponent(ps);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);

    xy::ParticleSystem::Definition music;
    music.loadFromFile("assets/particles/music.xyp", m_textureResource);
    entity = xy::Entity::create(m_messageBus);
    entity->addCommandCategories(Particle::Music);
    ps = music.createSystem(m_messageBus);
    entity->addComponent(ps);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);

    xy::ParticleSystem::Definition zz;
    zz.loadFromFile("assets/particles/zz.xyp", m_textureResource);
    entity = xy::Entity::create(m_messageBus);
    entity->addCommandCategories(Particle::Sleep);
    ps = zz.createSystem(m_messageBus);
    entity->addComponent(ps);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void WorldClientState::initUI()
{
    auto entity = xy::Entity::create(m_messageBus);
    auto dnc = xy::Component::create<DayNightCycle>(m_messageBus, m_scene.getSkyLight(), m_fontResource.get("assets/fonts/Clock.ttf"), true);
    entity->addComponent(dnc);
    entity->setPosition(20.f, 10.f);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);
}