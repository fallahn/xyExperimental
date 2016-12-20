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
#include <AttributeManager.hpp>
#include <TabComponent.hpp>
#include <TreeLightController.hpp>
#include <GameOverTab.hpp>
#include <ThinkBubble.hpp>
#include <TVAnimator.hpp>
#include <Background.hpp>

#include <xygine/App.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Reports.hpp>
#include <xygine/components/MeshDrawable.hpp>
#include <xygine/mesh/IQMBuilder.hpp>
#include <xygine/mesh/QuadBuilder.hpp>
#include <xygine/mesh/SphereBuilder.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/tilemap/Map.hpp>
#include <xygine/components/TileMapLayer.hpp>
#include <xygine/shaders/Tilemap.hpp>
#include <xygine/components/PointLight.hpp>
#include <xygine/tilemap/TileLayer.hpp>
#include <xygine/components/ParticleController.hpp>
#include <xygine/SysTime.hpp>

#include <xygine/postprocess/Blur.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>

namespace
{
    sf::Vector2f budSize(100.f, 200.f);
}

using namespace std::placeholders;

WorldClientState::WorldClientState(xy::StateStack& stateStack, Context context)
    : State                 (stateStack, context),
    m_messageBus            (context.appInstance.getMessageBus()),
    m_scene                 (m_messageBus),
    m_meshRenderer          ({ context.appInstance.getVideoSettings().VideoMode.width, context.appInstance.getVideoSettings().VideoMode.height }, m_scene),
    m_attribManager         (m_messageBus)
{
    launchLoadingScreen();
    m_scene.setView(context.defaultView);

    initMeshes();
    initMapData();
    initBud();
    initParticles();
    initUI();

    auto pp = xy::PostProcess::create<xy::PostBlur>();
    auto pptr = dynamic_cast<xy::PostBlur*>(pp.get());
    xy::PostProcess::MessageHandler mh;
    mh.id = xy::Message::UIMessage;
    mh.action = [pptr](const xy::Message& msg)
    {
        const auto& msgData = msg.getData<xy::Message::UIEvent>();
        if (msgData.type == xy::Message::UIEvent::MenuClosed)
        {
            pptr->setEnabled(false);
        }
        else if (msgData.type == xy::Message::UIEvent::MenuOpened)
        {
            pptr->setEnabled(true);
        }
    };
    pp->addMessageHandler(mh);
    m_scene.addPostProcess(pp);
    
    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::RequestState;
    msg->stateID = States::ID::Menu;

    quitLoadingScreen();
}

//public
bool WorldClientState::update(float dt)
{    
    m_attribManager.update(dt);
    m_scene.update(dt);
    m_meshRenderer.update();
    return false;
}

bool WorldClientState::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::Escape:
        case sf::Keyboard::P:
        case sf::Keyboard::Pause:
            requestStackPush(States::ID::Menu);
            break;
        }
    }
    else if (evt.type == sf::Event::MouseButtonReleased)
    {
        switch (evt.mouseButton.button)
        {
        case sf::Mouse::Left:
        {
            auto worldPos = xy::App::getMouseWorldPosition();
            auto msg = m_messageBus.post<Message::InterfaceEvent>(Message::Interface);
            msg->type = Message::InterfaceEvent::MouseClick;
            msg->positionX = worldPos.x;
            msg->positionY = worldPos.y;
        }
            break;
        default:break;
        }
    }
    else if (evt.type == sf::Event::MouseMoved)
    {
        auto worldPos = xy::App::getMouseWorldPosition();
        auto msg = m_messageBus.post<Message::InterfaceEvent>(Message::Interface);
        msg->type = Message::InterfaceEvent::MouseMoved;
        msg->positionX = worldPos.x;
        msg->positionY = worldPos.y;
    }
    return false;
}

void WorldClientState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
    m_meshRenderer.handleMessage(msg);
    m_attribManager.handleMessage(msg);

    if (msg.id == xy::Message::UIMessage)
    {
        const auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::ResizedWindow:
        {
            m_scene.setView(getContext().defaultView);
            //m_meshRenderer.setView(getContext().defaultView);
        }
        break;
        }
    }
    else if (msg.id == Message::Player)
    {
        const auto& data = msg.getData<Message::PlayerEvent>();
        if (data.action == Message::PlayerEvent::Died)
        {
            //add a menu
            auto& font = m_fontResource.get("assets/fonts/FallahnHand.ttf");
            auto tab = xy::Component::create<GameOverTab>(m_messageBus, font, m_textureResource, m_attribManager);
            auto entity = xy::Entity::create(m_messageBus);
            entity->setPosition(xy::DefaultSceneSize / 2.f);
            entity->addComponent(tab);
            m_scene.addEntity(entity, xy::Scene::Layer::UI);
        }
    }
    else if (msg.id == Message::System)
    {
        const auto& data = msg.getData<Message::SystemEvent>();
        switch (data.action)
        {
        default: break;
        case Message::SystemEvent::ResetGame:
            requestStackPop();
            requestStackPush(States::ID::Intro);
            break;
        case Message::SystemEvent::ToggleShadowMapping:
        {

        }
            break;
        }
    }
}

void WorldClientState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    rw.setView(getContext().defaultView);
    //rw.draw(m_meshRenderer);
#ifdef _DEBUG_
    //rw.setView(getContext().defaultView);
    //rw.draw(m_pathFinder);
#endif //_DEBUG_    
}

//private
void WorldClientState::initMeshes()
{
    m_meshRenderer.setView(getContext().defaultView);
    m_meshRenderer.setFOV(25.f);
    
    //TODO just search the dir for files and attempt to load with corresponding materials
    m_textureResource.setFallbackColour(sf::Color::Black);
    const auto& maskTex = m_textureResource.get("fallback_mask");

    xy::IQMBuilder haus("assets/models/haus.iqm");
    m_meshRenderer.loadModel(Mesh::Haus, haus);

    auto& hausMat = m_meshRenderer.addMaterial(Material::Haus, xy::Material::TexturedBumped, true);
    hausMat.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/textures/haus_diffuse.png") });
    hausMat.addProperty({ "u_normalMap", m_textureResource.get("assets/images/textures/haus_normal.png") });
    hausMat.addProperty({ "u_maskMap", m_textureResource.get("assets/images/textures/haus_mask.png") });
    hausMat.getRenderPass(xy::RenderPass::ID::Default)->setCullFace(xy::CullFace::Front);

    auto hausModel = m_meshRenderer.createModel(Mesh::Haus, m_messageBus);
    hausModel->setBaseMaterial(hausMat);

    /*xy::IQMBuilder background("assets/models/background.iqm");
    m_meshRenderer.loadModel(Mesh::Background, background);

    auto& backMat = m_meshRenderer.addMaterial(Material::Background, xy::Material::TexturedBumped)
    backMat.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/textures/background_diffuse.png") });
    backMat.addProperty({ "u_normalMap", m_textureResource.get("assets/images/textures/haus_normal.png") });

    auto backgroundModel = m_meshRenderer.createModel(Mesh::Background, m_messageBus);
    backgroundModel->setBaseMaterial(backMat);*/

    xy::IQMBuilder furniture("assets/models/furniture.iqm");
    m_meshRenderer.loadModel(Mesh::Furniture, furniture);

    auto& furnitureMat = m_meshRenderer.addMaterial(Material::Furniture, xy::Material::TexturedBumped, true);
    furnitureMat.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/textures/furniture_diffuse.png") });
    furnitureMat.addProperty({ "u_normalMap", m_textureResource.get("assets/images/textures/furniture_normal.png") });
    furnitureMat.addProperty({ "u_maskMap", maskTex });
    furnitureMat.getRenderPass(xy::RenderPass::ID::Default)->setCullFace(xy::CullFace::Front);
    furnitureMat.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);

    auto furnitureModel = m_meshRenderer.createModel(Mesh::Furniture, m_messageBus);
    furnitureModel->setBaseMaterial(furnitureMat);

    xy::IQMBuilder moreFurniture("assets/models/more_furniture.iqm");
    m_meshRenderer.loadModel(Mesh::MoreFurniture, moreFurniture);

    auto& moreFurnitureMat = m_meshRenderer.addMaterial(Material::MoreFurniture, xy::Material::TexturedBumped, true);
    moreFurnitureMat.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/textures/furniture2_diffuse.png") });
    moreFurnitureMat.addProperty({ "u_normalMap", m_textureResource.get("assets/images/textures/furniture2_normal.png") });
    moreFurnitureMat.addProperty({ "u_maskMap", maskTex });
    moreFurnitureMat.getRenderPass(xy::RenderPass::ID::Default)->setCullFace(xy::CullFace::Front);
    moreFurnitureMat.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);

    auto moreFurnitureModel = m_meshRenderer.createModel(Mesh::MoreFurniture, m_messageBus);
    moreFurnitureModel->setBaseMaterial(moreFurnitureMat);

    xy::IQMBuilder thirdFurniture("assets/models/furniture_the_third.iqm");
    m_meshRenderer.loadModel(Mesh::ThirdFurniture, thirdFurniture);

    auto tvAnimator = xy::Component::create<TVAnimator>(m_messageBus);

    auto& thirdFurnitureMat = m_meshRenderer.addMaterial(Material::ThirdFurniture, xy::Material::TexturedBumped, true);
    thirdFurnitureMat.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/textures/furniture3_diffuse.png") });
    thirdFurnitureMat.addProperty({ "u_normalMap", m_textureResource.get("assets/images/textures/furniture3_normal.png") });
    thirdFurnitureMat.addProperty({ "u_maskMap", tvAnimator->getMaskTexture() });
    thirdFurnitureMat.getRenderPass(xy::RenderPass::ID::Default)->setCullFace(xy::CullFace::Front);
    thirdFurnitureMat.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);

    auto thirdFurnitureModel = m_meshRenderer.createModel(Mesh::ThirdFurniture, m_messageBus);
    thirdFurnitureModel->setBaseMaterial(thirdFurnitureMat);

    xy::IQMBuilder walls("assets/models/walls.iqm");
    m_meshRenderer.loadModel(Mesh::Walls, walls);

    auto& wallMat = m_meshRenderer.addMaterial(Material::Walls, xy::Material::Textured, true);
    wallMat.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/textures/walls_diffuse.png") });
    wallMat.getRenderPass(xy::RenderPass::ID::Default)->setCullFace(xy::CullFace::Front);

    auto wallModel = m_meshRenderer.createModel(Mesh::Walls, m_messageBus);
    wallModel->setBaseMaterial(wallMat);

    auto entity = xy::Entity::create(m_messageBus);

    //christmas decs
    if (xy::SysTime::now().months() == 12)
    {
        xy::IQMBuilder tree("assets/models/tree.iqm");
        m_meshRenderer.loadModel(Mesh::Tree, tree);

        auto treeModel = m_meshRenderer.createModel(Mesh::Tree, m_messageBus);
        treeModel->setBaseMaterial(thirdFurnitureMat);
        entity->addComponent(treeModel);

        xy::IQMBuilder lights("assets/models/lights.iqm");
        m_meshRenderer.loadModel(Mesh::Lights, lights);

        auto& lightMat = m_meshRenderer.addMaterial(Material::Lights, xy::Material::Description::Coloured);
        lightMat.addProperty({ "u_maskColour", sf::Color::Blue });
        lightMat.addProperty({ "u_colour", sf::Color(235, 235, 180) });
        lightMat.getRenderPass(xy::RenderPass::ID::Default)->setCullFace(xy::CullFace::Front);

        auto lightModel = m_meshRenderer.createModel(Mesh::Lights, m_messageBus);
        lightModel->setBaseMaterial(lightMat);
        entity->addComponent(lightModel);

        auto lightController = xy::Component::create<TreeLightController>(m_messageBus, lightMat);
        entity->addComponent(lightController);
    }

    entity->addComponent(hausModel);
    //entity->addComponent(backgroundModel);
    entity->addComponent(furnitureModel);
    entity->addComponent(moreFurnitureModel);
    entity->addComponent(tvAnimator);
    entity->addComponent(thirdFurnitureModel);
    entity->addComponent(wallModel);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    m_scene.addEntity(entity, xy::Scene::BackFront);



    //drawable containing mesh renderer output
    entity = xy::Entity::create(m_messageBus);
    auto md = m_meshRenderer.createDrawable(m_messageBus);
    entity->addComponent(md);
    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);

    //quad for player
    xy::QuadBuilder qb(budSize);
    m_meshRenderer.loadModel(Mesh::Bud, qb);
    
    m_textureResource.setFallbackColour({ 127, 127, 255 });

    auto& budMat = m_meshRenderer.addMaterial(Material::Bud, xy::Material::TexturedBumped, true, true);
    budMat.addProperty({ "u_normalMap", m_textureResource.get("fallback") });
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
                        auto light = xy::Component::create<xy::PointLight>(m_messageBus, 440.f, 270.f, sf::Color(255,233,240));
                        light->setDepth(160.f);
                        light->setIntensity(0.f);
                        light->enableShadowCasting(true);

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
                        task.worldPosition.x = static_cast<float>(task.position.x * map.getTileSize().x);
                        task.worldPosition.y = static_cast<float>((task.position.y + 1) * map.getTileSize().y);
                        task.worldPosition += mapOffset;
                        
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
                                else if (propName == "animation")
                                {
                                    task.animationID = prop.getIntValue();
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!m_pathFinder.hasData())
        {
            xy::Logger::log("No navigation data has been loaded!", xy::Logger::Type::Error);
        }
    }
    else
    {
        xy::Logger::log("Failed to open map data", xy::Logger::Type::Error, xy::Logger::Output::All);
    }

    //draws background
    auto& bgTex = m_textureResource.get("assets/images/textures/background.png");
    bgTex.setRepeated(true);
    bgTex.setSmooth(true);
    auto bg = xy::Component::create<Background>(m_messageBus, bgTex);
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(bg);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);
}

void WorldClientState::initBud()
{
    auto controller = xy::Component::create<BudController>(m_messageBus, m_attribManager, m_pathFinder, m_tasks, m_textureResource.get("assets/images/sprites/bud.png"));

    auto& material = m_meshRenderer.getMaterial(Material::Bud);
    material.addProperty({ "u_diffuseMap", controller->getTexture() });

    auto dwb = m_meshRenderer.createModel(Mesh::Bud, m_messageBus);
    dwb->setBaseMaterial(material);
    dwb->setPosition({ 0.f, -((budSize.y / 2.f) - 6.f), 4.f });

    const auto& thinkTex = m_textureResource.get("assets/images/ui/think_bubble.png");
    auto thinkBubble = xy::Component::create<ThinkBubble>(m_messageBus, thinkTex);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(dwb);
    entity->addComponent(controller);
    entity->addComponent(thinkBubble);

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
    ps->followParent(true);
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

namespace
{
    const float tabWidth = 450.f;
    const float tabHeight = 180.f;
}

#include <TimeTab.hpp>
#include <PersonalTab.hpp>
#include <HouseholdTab.hpp>

void WorldClientState::initUI()
{
    //do tabs first so they appear behind
    auto leftTab = xy::Component::create<TabComponent>(m_messageBus, sf::Vector2f(tabWidth, xy::DefaultSceneSize.y),
        TabComponent::Direction::Horizontal, m_textureResource.get("assets/images/ui/large_tab.png"));
    auto personalTab = xy::Component::create<PersonalTab>(m_messageBus, m_fontResource, m_textureResource, m_attribManager);
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(leftTab);
    entity->addComponent(personalTab);
    entity->setPosition(-tabWidth, 0.f);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);
    
    auto rightTab = xy::Component::create<TabComponent>(m_messageBus, sf::Vector2f(tabWidth, xy::DefaultSceneSize.y),
        TabComponent::Direction::Horizontal, m_textureResource.get("assets/images/ui/large_tab.png"));
    auto householdTab = xy::Component::create<HouseholdTab>(m_messageBus, m_fontResource, m_textureResource, m_attribManager);
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(rightTab);
    entity->addComponent(householdTab);
    entity->setPosition(xy::DefaultSceneSize.x + tabWidth, 0.f);
    entity->setScale(-1.f, 1.f);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);

    auto topTab = xy::Component::create<TabComponent>(m_messageBus, sf::Vector2f(xy::DefaultSceneSize.x, tabHeight),
        TabComponent::Direction::Vertical, m_textureResource.get("assets/images/ui/top_tab.png"));
    auto timeInfo = xy::Component::create<TimeTab>(m_messageBus, m_fontResource, m_textureResource, m_attribManager);
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(topTab);
    entity->addComponent(timeInfo);
    entity->setPosition(0.f, -tabHeight);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);

    entity = xy::Entity::create(m_messageBus);
    auto dnc = xy::Component::create<DayNightCycle>(m_messageBus, m_scene.getSkyLight(), true);
    entity->addComponent(dnc);
    entity->setPosition(20.f, 10.f);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);
}