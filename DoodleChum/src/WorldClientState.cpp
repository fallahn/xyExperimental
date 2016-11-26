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

#include <xygine/App.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Reports.hpp>
#include <xygine/components/MeshDrawable.hpp>
#include <xygine/mesh/IQMBuilder.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/tilemap/Map.hpp>
#include <xygine/components/TileMapLayer.hpp>
#include <xygine/shaders/Tilemap.hpp>
#include <xygine/components/PointLight.hpp>

#include <xygine/postprocess/ChromeAb.hpp>
#include <xygine/postprocess/Antique.hpp>

namespace
{

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
            m_meshRenderer.setView(getContext().defaultView);
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
}

//private
void WorldClientState::initMeshes()
{
    m_meshRenderer.setView(getContext().defaultView);

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
}

void WorldClientState::initMapData()
{
    xy::tmx::Map map;
    if (map.load("assets/mapdata/map.tmx"))
    {
        m_shaderResource.preload(xy::Shader::Tilemap, xy::Shader::tmx::vertex, xy::Shader::tmx::fragment);

        sf::Vector2f mapOffset = (xy::DefaultSceneSize / 2.f) - sf::Vector2f(map.getBounds().width / 2.f, map.getBounds().height / 2.f);

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
            }
            else if (l->getType() == xy::tmx::Layer::Type::Object)
            {
                if (l->getName() == "lights")
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
            }
        }
    }
    else
    {
        xy::Logger::log("Failed to open map data", xy::Logger::Type::Error, xy::Logger::Output::All);
    }
}

void WorldClientState::initUI()
{
    auto entity = xy::Entity::create(m_messageBus);
    auto dnc = xy::Component::create<DayNightCycle>(m_messageBus, m_scene.getSkyLight(), m_fontResource.get("spanish chicken"), true);
    entity->addComponent(dnc);
    entity->setPosition(20.f, 10.f);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);
}