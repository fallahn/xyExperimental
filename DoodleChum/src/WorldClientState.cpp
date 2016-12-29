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
#include <WallClock.hpp>
#include <CatController.hpp>
#include <Vacuum.hpp>

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
#include <xygine/components/AudioSource.hpp>
#include <xygine/components/SoundPlayer.hpp>

#include <xygine/postprocess/Blur.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>

namespace
{
    const sf::Vector2f budSize(100.f, 200.f);
    const sf::Vector2f catSize(80.f, 80.f);
    const sf::Vector2f clockSize(80.f, 194.f);
    const sf::Vector2f vacuumSize(127.f, 78.f);
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
    initCat();
    initBud();
    initParticles();
    initUI();
    initSounds();

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

    switch (msg.id)
    {
    default: break;

    case xy::Message::UIMessage:
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
        break;
    case Message::Player:
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
        break;
        case Message::System:
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
        break;
        case Message::Animation:
        {
            const auto& msgData = msg.getData<Message::AnimationEvent>();
            if (msgData.id == Message::AnimationEvent::VacuumStill
                || msgData.id == Message::AnimationEvent::VacuumWalk)
            {
                xy::Command cmd;
                cmd.category = Command::Vacuum;
                cmd.action = [](xy::Entity& entity, float)
                {
                    //move the vacuum so it's visible
                    entity.setPosition(0.f, 0.f);
                };
                m_scene.sendCommand(cmd);
            }
            else if(msgData.id == Message::AnimationEvent::Left)
            {
                //put the vacuum away
                xy::Command cmd;
                cmd.category = Command::Vacuum;
                cmd.action = [](xy::Entity& entity, float)
                {
                    entity.setPosition(-xy::DefaultSceneSize * 2.f);
                };
                m_scene.sendCommand(cmd);
            }
        }
            break;
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
    m_textureResource.get("assets/images/textures/haus_diffuse.png").setSmooth(true);

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

    //quad for vacuum
    xy::QuadBuilder vacuumQuad(vacuumSize);
    m_meshRenderer.loadModel(Mesh::Vacuum, vacuumQuad);
    m_meshRenderer.addMaterial(Material::Vaccum, xy::Material::Textured, true, true);
    
    //quad for player
    xy::QuadBuilder qb(budSize);
    m_meshRenderer.loadModel(Mesh::Bud, qb);
    
    m_textureResource.setFallbackColour({ 127, 127, 255 });
    /*auto& budMat = */m_meshRenderer.addMaterial(Material::Bud, xy::Material::Textured/*Bumped*/, true, true);
    //budMat.addProperty({ "u_normalMap", m_textureResource.get("fallback") });

    //quad for cat
    xy::QuadBuilder cq(catSize);
    m_meshRenderer.loadModel(Mesh::Cat, cq);
    /*auto& catMat = */m_meshRenderer.addMaterial(Material::Cat, xy::Material::Textured, true, true);

    //quad for clock
    xy::QuadBuilder clockQuad(clockSize);
    m_meshRenderer.loadModel(Mesh::Clock, clockQuad);
    m_meshRenderer.addMaterial(Material::Clock, xy::Material::Textured, true, true);
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
                std::function<void(const std::vector<xy::tmx::Object>&, std::vector<TaskData>&)> parseTasks =
                    [&map, &mapOffset](const std::vector<xy::tmx::Object>& objs, std::vector<TaskData>& taskList)
                {
                    for (const auto& obj : objs)
                    {
                        taskList.emplace_back();
                        auto& task = taskList.back();
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
                };
                
                
                const auto& name = l->getName();
                const auto& objs = dynamic_cast<xy::tmx::ObjectGroup*>(l.get())->getObjects();
                if (name == "lights")
                {
                    for (const auto& o : objs)
                    {
                        auto light = xy::Component::create<xy::PointLight>(m_messageBus, 540.f, 270.f, sf::Color(255,233,240));
                        light->setDepth(200.f);
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
                    parseTasks(objs, m_tasks);
                }
                else if (name == "cat")
                {
                    parseTasks(objs, m_catTasks);
                }
                else if(name == "idle")
                {
                    parseTasks(objs, m_idleTasks);
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

    //draws the wall clock
    auto wc = xy::Component::create<WallClock>(m_messageBus, m_textureResource.get("assets/images/sprites/clock.png"));
    auto dwb = m_meshRenderer.createModel(Mesh::Clock, m_messageBus);
    auto& material = m_meshRenderer.getMaterial(Material::Clock);
    material.addProperty({ "u_diffuseMap", wc->getTexture() });
    dwb->setBaseMaterial(material);
    dwb->setPosition({ 0.f, 0.f, 1.f });
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(wc);
    entity->addComponent(dwb);
    entity->setPosition(660.f, 540.f);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);
}

void WorldClientState::initBud()
{
    //vacuum cleaner - need to create this first for draw order    
    auto& vacMat = m_meshRenderer.getMaterial(Material::Vaccum);
    vacMat.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/sprites/vacuum.png") });
    
    auto vacDrb = m_meshRenderer.createModel(Mesh::Vacuum, m_messageBus);
    vacDrb->setBaseMaterial(vacMat);
    vacDrb->setPosition({ 0.f, -(vacuumSize.y / 2.f) + 6.f, 3.f });
    
    const auto& audioSettings = getContext().appInstance.getAudioSettings();

    auto vacSound = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
    vacSound->setSound("assets/sound/fx/vacuum_loop.wav");
    vacSound->setVolume(audioSettings.muted ? 0.f : audioSettings.volume);
    vacSound->setFadeInTime(1.f);
    vacSound->setFadeOutTime(1.f);
    vacSound->addMessageHandler(getVolumeHandler());

    auto vacSoundEnd = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
    vacSoundEnd->setSound("assets/sound/fx/vacuum_end.wav");
    vacSoundEnd->setVolume(audioSettings.muted ? 0.f : audioSettings.volume);
    vacSoundEnd->setFadeInTime(0.1f);
    vacSoundEnd->setFadeOutTime(0.1f);
    vacSoundEnd->addMessageHandler(getVolumeHandler());

    auto vacController = xy::Component::create<VacuumController>(m_messageBus);

    auto vacEnt = xy::Entity::create(m_messageBus);
    vacEnt->addCommandCategories(Command::Vacuum);
    vacEnt->addComponent(vacDrb);
    vacEnt->addComponent(vacSound);
    vacEnt->addComponent(vacSoundEnd);
    vacEnt->addComponent(vacController);
    vacEnt->setPosition(-xy::DefaultSceneSize * 2.f); //put off screen to start with
    vacEnt->setOrigin(-vacuumSize.x / 2.5f, 0.f);
    
    //bob
    auto controller = xy::Component::create<BudController>(m_messageBus, m_attribManager, m_pathFinder,
        m_tasks, m_idleTasks, m_textureResource.get("assets/images/sprites/bud.png"));

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

    entity->addChild(vacEnt);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void WorldClientState::initCat()
{
    auto controller = xy::Component::create<CatController>(m_messageBus, m_pathFinder, m_catTasks, m_textureResource.get("assets/images/sprites/pussy.png"));
    
    auto& material = m_meshRenderer.getMaterial(Material::Cat);
    material.addProperty({ "u_diffuseMap", controller->getTexture() });

    auto dwb = m_meshRenderer.createModel(Mesh::Cat, m_messageBus);
    dwb->setBaseMaterial(material);
    dwb->setPosition({ 0.f, (-catSize.y / 2.f) + 6.f, 2.f });

    xy::ParticleSystem::Definition zz;
    zz.loadFromFile("assets/particles/zz_small.xyp", m_textureResource);
    auto ps = zz.createSystem(m_messageBus);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(controller);
    entity->addComponent(dwb);
    entity->addComponent(ps);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void WorldClientState::initParticles()
{
    xy::ParticleSystem::Definition steam;
    steam.loadFromFile("assets/particles/steam.xyp", m_textureResource);

    auto showerSound = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
    showerSound->setSound("assets/sound/fx/shower.ogg");
    showerSound->setFadeInTime(0.2f);
    showerSound->addMessageHandler(getVolumeHandler());

    auto entity = xy::Entity::create(m_messageBus);
    entity->addCommandCategories(Particle::Steam);
    auto ps = steam.createSystem(m_messageBus);
    entity->addComponent(ps);
    entity->addComponent(showerSound);
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

namespace
{
    const std::string musicPath("assets/sound/music/");
    const std::string pianoPath("assets/sound/piano/");
    const std::string TVPath("assets/sound/fx/tv/");
}

void WorldClientState::initSounds()
{
    const auto& audioSettings = getContext().appInstance.getAudioSettings();
    
    //entity to play music files
    auto entity = xy::Entity::create(m_messageBus);
    entity->addCommandCategories(Command::MusicPlayer);
    auto fileList = xy::FileSystem::listFiles(musicPath);
    auto count = 0;
    for (const auto& f : fileList)
    {
        if (xy::FileSystem::getFileExtension(f) == ".ogg")
        {
            auto music = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
            music->setSound(musicPath + f, xy::AudioSource::Mode::Stream);
            music->setVolume(audioSettings.muted ? 0.f : audioSettings.volume);
            music->setFadeInTime(3.f);
            music->setFadeOutTime(3.f);
            music->addMessageHandler(getVolumeHandler());
            auto musicPtr = music.get();

            xy::Component::MessageHandler mh;
            mh.id = Message::TaskCompleted;
            mh.action = [musicPtr](xy::Component*, const xy::Message& msg)
            {
                const auto& data = msg.getData<Message::TaskEvent>();
                if (data.taskName == Message::TaskEvent::PlayMusic)
                {
                    musicPtr->pause();
                }
            };
            music->addMessageHandler(mh);
            entity->addComponent(music);
            count++;
        }
    }

    if(count > 0) m_scene.addEntity(entity, xy::Scene::Layer::UI);

    //entity to play piano music
    entity = xy::Entity::create(m_messageBus);
    entity->addCommandCategories(Command::PianoPlayer);
    fileList = xy::FileSystem::listFiles(pianoPath);
    count = 0;
    for (const auto& f : fileList)
    {
        if (xy::FileSystem::getFileExtension(f) == ".ogg")
        {
            auto music = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
            music->setSound(pianoPath + f, xy::AudioSource::Mode::Stream);
            music->setVolume(audioSettings.muted ? 0.f : audioSettings.volume);
            music->setFadeInTime(1.f);
            music->setFadeOutTime(1.f);
            music->addMessageHandler(getVolumeHandler());
            auto musicPtr = music.get();

            xy::Component::MessageHandler mh;
            mh.id = Message::TaskCompleted;
            mh.action = [musicPtr](xy::Component*, const xy::Message& msg)
            {
                const auto& data = msg.getData<Message::TaskEvent>();
                if (data.taskName == Message::TaskEvent::PlayPiano)
                {
                    musicPtr->stop();
                }
            };
            music->addMessageHandler(mh);
            entity->addComponent(music);
            count++;
        }
    }
    if (count > 0) m_scene.addEntity(entity, xy::Scene::Layer::UI);


    //single entity to play loops sounds such as ambience / clock
    auto daySound = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
    daySound->addMessageHandler(getVolumeHandler());
    daySound->setSound("assets/sound/fx/day_ambience.ogg", xy::AudioSource::Mode::Stream);
    daySound->setVolume(audioSettings.muted ? 0.f : audioSettings.volume);
    daySound->setFadeInTime(1.f);
    daySound->play(true);
    auto dsPtr = daySound.get();

    xy::Component::MessageHandler mh;
    mh.id = Message::TimeOfDay;
    mh.action = [dsPtr, &audioSettings](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::TODEvent>();
        if (!audioSettings.muted)
        {
            dsPtr->setVolume(100.f * audioSettings.volume * data.sunIntensity);
        }
    };
    daySound->addMessageHandler(mh);

    auto nightSound = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
    nightSound->addMessageHandler(getVolumeHandler());
    nightSound->setSound("assets/sound/fx/night_ambience.ogg", xy::AudioSource::Mode::Stream);
    nightSound->setVolume(audioSettings.muted ? 0.f : audioSettings.volume);
    nightSound->setFadeInTime(1.f);
    nightSound->play(true);
    auto nsPtr = nightSound.get();

    mh.action = [nsPtr, &audioSettings](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::TODEvent>();
        if (!audioSettings.muted)
        {
            nsPtr->setVolume(100.f * audioSettings.volume * (1.f - data.sunIntensity));
        }
    };
    nightSound->addMessageHandler(mh);

    auto printerSound = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
    printerSound->addMessageHandler(getVolumeHandler());
    printerSound->setSound("assets/sound/ui/printer.wav");
    printerSound->setVolume(audioSettings.muted ? 0.f : audioSettings.volume);
    auto psPtr = printerSound.get();

    mh.id = Message::Interface;
    mh.action = [psPtr](xy::Component*, const xy::Message& msg)
    {
        const auto& msgData = msg.getData<Message::InterfaceEvent>();
        if (msgData.type == Message::InterfaceEvent::PrintBegin)
        {
            psPtr->play(true);
        }
        else if (msgData.type == Message::InterfaceEvent::PrintEnd)
        {
            psPtr->stop();
        }
    };
    printerSound->addMessageHandler(mh);

    auto scrollSound = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
    scrollSound->addMessageHandler(getVolumeHandler());
    scrollSound->setSound("assets/sound/ui/printer_scroll.wav");
    scrollSound->setVolume(audioSettings.muted ? 0.f : audioSettings.volume);
    auto ssPtr = scrollSound.get();

    mh.action = [ssPtr](xy::Component*, const xy::Message& msg)
    {
        const auto& msgData = msg.getData<Message::InterfaceEvent>();
        if (msgData.type == Message::InterfaceEvent::PrintScroll)
        {
            ssPtr->play(true);
        }
        else if (msgData.type == Message::InterfaceEvent::PrintEnd)
        {
            ssPtr->stop();
        }
    };
    scrollSound->addMessageHandler(mh);

    auto snoreSound = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
    snoreSound->setSound("assets/sound/fx/snoring.ogg");
    snoreSound->setFadeInTime(1.f);
    snoreSound->setFadeOutTime(1.f);
    snoreSound->setVolume(audioSettings.muted ? 0.f : audioSettings.volume);
    snoreSound->addMessageHandler(getVolumeHandler());
    auto snPtr = snoreSound.get();

    mh.id = Message::Animation;
    mh.action = [snPtr](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::AnimationEvent>();
        if (data.id & Message::CatAnimMask) return;
        else if (data.id == Message::AnimationEvent::Sleep)
        {
            snPtr->play(true);
        }
        else
        {
            snPtr->stop();
        }
    };
    snoreSound->addMessageHandler(mh);

    //and a soundplayer component to handle one-shot effects
    auto soundPlayer = xy::Component::create<xy::SoundPlayer>(m_messageBus, m_soundResource);
    soundPlayer->setMasterVolume(audioSettings.muted ? 0.f : audioSettings.volume);
    auto playerPtr = soundPlayer.get();
    soundPlayer->preCache(Sound::MenuOpen, "assets/sound/ui/menu_open.wav");
    soundPlayer->preCache(Sound::MenuClose, "assets/sound/ui/menu_close.wav");

    mh.id = xy::Message::UIMessage;
    mh.action = [playerPtr, &audioSettings](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<xy::Message::UIEvent>();
        switch(data.type)
        {
        case xy::Message::UIEvent::MenuClosed:
            playerPtr->playSound(Sound::MenuClose, 0.f, 0.f);
            break;
        case xy::Message::UIEvent::MenuOpened:
            playerPtr->playSound(Sound::MenuOpen, 0.f, 0.f);
            break;
        case xy::Message::UIEvent::RequestAudioMute:
            playerPtr->setMasterVolume(0.f);
            break;
        case xy::Message::UIEvent::RequestAudioUnmute:
            playerPtr->setMasterVolume(data.value);
            break;
        case xy::Message::UIEvent::RequestVolumeChange:
            if(!audioSettings.muted) playerPtr->setMasterVolume(data.value);
            break;
        default: break;
        }
    };
    soundPlayer->addMessageHandler(mh);

    soundPlayer->preCache(Sound::ButtonClick, "assets/sound/ui/button_click.wav");
    soundPlayer->preCache(Sound::TabOpen, "assets/sound/ui/tab_open.wav");
    soundPlayer->preCache(Sound::NoMoney, "assets/sound/ui/no_money.wav");
    mh.id = Message::Interface;
    mh.action = [playerPtr](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::InterfaceEvent>();
        auto centre = xy::DefaultSceneSize / 2.f;
        if (data.type == Message::InterfaceEvent::TabToggled)
        {
            playerPtr->playSound(Sound::TabOpen, centre.x, centre.y);
        }
        /*else if (data.type == Message::InterfaceEvent::ButtonClick)
        {
            playerPtr->playSound(Sound::ButtonClick, centre.x, centre.y);
        }*/
        else if (data.type == Message::InterfaceEvent::NoMoney)
        {
            playerPtr->playSound(Sound::NoMoney, centre.x, centre.y);
        }
    };
    soundPlayer->addMessageHandler(mh);

    soundPlayer->preCache(Sound::GotMoney, "assets/sound/ui/payday.wav");
    mh.id = Message::Attribute;
    mh.action = [playerPtr](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::AttribEvent>();
        if (data.action == Message::AttribEvent::SpentMoney)
        {
            playerPtr->playSound(Sound::ButtonClick, 0.f, 0.f);
        }
        else if (data.action == Message::AttribEvent::GotPaid)
        {
            //KERCHING!
            playerPtr->playSound(Sound::GotMoney, 0.f, 0.f);
        }
    };
    soundPlayer->addMessageHandler(mh);

    soundPlayer->preCache(Sound::ToiletFlush, "assets/sound/fx/flush.wav");
    mh.id = Message::TaskCompleted;
    mh.action = [playerPtr, this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::TaskEvent>();
        switch (data.taskName)
        {
        default: break;
        case Message::TaskEvent::Poop:
            playerPtr->playSound(Sound::ToiletFlush, m_tasks[Message::TaskEvent::Poop].worldPosition.x,
                                                    m_tasks[Message::TaskEvent::Poop].worldPosition.y);
            break;
        }
    };
    soundPlayer->addMessageHandler(mh);

    soundPlayer->preCache(Sound::RemoteClick, "assets/sound/fx/remote.wav");
    mh.id = Message::Animation;
    mh.action = [playerPtr](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::AnimationEvent>();
        switch (data.id)
        {
        default:break;
        case Message::AnimationEvent::TV:
            playerPtr->playSound(Sound::RemoteClick, 0.f, 0.f);
            break;
        }
    };
    soundPlayer->addMessageHandler(mh);

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(daySound);
    entity->addComponent(nightSound);
    entity->addComponent(printerSound);
    entity->addComponent(scrollSound);
    entity->addComponent(snoreSound);
    entity->addComponent(soundPlayer);

    entity->setPosition(xy::DefaultSceneSize / 2.f);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);


    //tv sounds are on their own entity so can be updated by the animation controller
    entity = xy::Entity::create(m_messageBus);
    entity->addCommandCategories(Command::TVAudio);
    fileList = xy::FileSystem::listFiles(TVPath);
    std::random_shuffle(std::begin(fileList), std::end(fileList));
    count = 0;
    for (const auto& f : fileList)
    {
        if (xy::FileSystem::getFileExtension(f) == ".ogg")
        {
            auto music = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
            music->setSound(TVPath + f, xy::AudioSource::Mode::Stream);
            music->setVolume(audioSettings.muted ? 0.f : audioSettings.volume);
            music->setFadeInTime(0.1f);
            music->setFadeOutTime(0.1f);
            music->addMessageHandler(getVolumeHandler());
            auto musicPtr = music.get();

            xy::Component::MessageHandler mh;
            mh.id = Message::TaskCompleted;
            mh.action = [musicPtr](xy::Component*, const xy::Message& msg)
            {
                const auto& data = msg.getData<Message::TaskEvent>();
                if (data.taskName == Message::TaskEvent::WatchTV)
                {
                    musicPtr->stop();
                }
            };
            music->addMessageHandler(mh);
            entity->addComponent(music);
            count++;
        }
    }
    if (count > 0) m_scene.addEntity(entity, xy::Scene::Layer::UI);
}

xy::Component::MessageHandler WorldClientState::getVolumeHandler()
{
    const auto& audioSettings = getContext().appInstance.getAudioSettings();
    
    xy::Component::MessageHandler volumeHandler;
    volumeHandler.id = xy::Message::UIMessage;
    volumeHandler.action =
        [&audioSettings](xy::Component* component, const xy::Message& msg)
    {
        const auto& data = msg.getData<xy::Message::UIEvent>();
        auto* playerPtr = dynamic_cast<xy::AudioSource*>(component);
        switch (data.type)
        {
        case xy::Message::UIEvent::RequestAudioMute:
            playerPtr->setVolume(0.f);
            break;
        case xy::Message::UIEvent::RequestAudioUnmute:
            playerPtr->setVolume(data.value * 100.f);
            break;
        case xy::Message::UIEvent::RequestVolumeChange:
            if (!audioSettings.muted) playerPtr->setVolume(data.value * 100.f);
            break;
        default: break;
        }
    };

    return std::move(volumeHandler);
}