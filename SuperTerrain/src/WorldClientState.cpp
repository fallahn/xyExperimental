/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

SuperTerrain - Zlib license.

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
#include <TerrainComponent.hpp>
#include <PlayerController.hpp>
#include <CameraController.hpp>
#include <PacketIDs.hpp>

#include <xygine/App.hpp>
#include <xygine/Command.hpp>
#include <xygine/components/Camera.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Reports.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Sleep.hpp>

namespace
{
    const int playerID = 300;

    //xy::Camera* playerCamera = nullptr;
}

using namespace std::placeholders;

WorldClientState::WorldClientState(xy::StateStack& stateStack, Context context)
    : State                 (stateStack, context),
    m_messageBus            (context.appInstance.getMessageBus()),
    m_scene                 (m_messageBus),
    m_broadcastAccumulator  (0.f)
{
    launchLoadingScreen();
    m_scene.setView(context.defaultView);

    init();

    //TODO option to not start if connecting to remote server
    m_server.start();
    sf::sleep(sf::seconds(1.f));

    m_packetHandler = std::bind(&WorldClientState::handlePacket, this, _1, _2, _3);
    m_connection.setPacketHandler(m_packetHandler);
    m_connection.setServerInfo({ "127.0.0.1" }, xy::Network::ServerPort);
    m_connection.connect();

    quitLoadingScreen();
}

//public
bool WorldClientState::update(float dt)
{
    sf::Uint32 input = 0;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) input |= st::PlayerController::Left;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) input |= st::PlayerController::Right;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) input |= st::PlayerController::Forward;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) input |= st::PlayerController::Back;
    //TODO we need entity rotation, or mouse position

    const float sendRate = 1.f / m_connection.getSendRate();
    m_broadcastAccumulator += m_broadcastClock.restart().asSeconds();
    while (m_broadcastAccumulator >= sendRate)
    {
        m_playerInput.clientID = m_connection.getClientID();
        m_playerInput.timestamp = m_connection.getTime().asMilliseconds();
        m_playerInput.input = input;

        m_broadcastAccumulator -= sendRate;
        sf::Packet packet;
        packet << xy::PacketID(PacketID::PlayerInput) << m_playerInput;
        m_connection.send(packet);

        m_playerInput.counter++;
    }
    
    m_server.update(dt);
    m_connection.update(dt);
    

    //do client side prediction
    xy::Command cmd;
    cmd.category = playerID;
    cmd.action = [this, &input](xy::Entity& entity, float dt)
    {
        //TODO move this to player calc
        auto direction = xy::App::getMouseWorldPosition() - entity.getWorldPosition();
        auto angle = xy::Util::Vector::rotation(direction);
        input |= (static_cast<sf::Int16>(angle) << 16);

        entity.getComponent<st::PlayerController>()->setInput(input);
    };
    m_scene.sendCommand(cmd);
    
    m_scene.update(dt);
    return false;
}

bool WorldClientState::handleEvent(const sf::Event& evt)
{
    return false;
}

void WorldClientState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);

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
            //m_scene.setView(getContext().defaultView);
        }
        break;
        }
    }
}

void WorldClientState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
}

//private
void WorldClientState::handlePacket(xy::Network::PacketType packetType, sf::Packet& packet, xy::Network::ClientConnection* connection)
{
    switch (packetType)
    {
    default: break;
    case xy::Network::Connect:
    {
        sf::Packet newPacket;
        newPacket << xy::PacketID(PacketID::PlayerDetails) << m_connection.getClientID() << "Player";
        connection->send(newPacket, true);
    }
    break;

    }
}

void WorldClientState::init()
{
    auto tc = xy::Component::create<TerrainComponent>(m_messageBus, getContext().appInstance);
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(tc);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    auto dwb = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(m_messageBus);
    dwb->getDrawable().setRadius(20.f);
    dwb->getDrawable().setOrigin(20.f, 20.f);
    dwb->getDrawable().setFillColor(sf::Color::Red);
    dwb->getDrawable().setPointCount(3);
    dwb->getDrawable().setRotation(90.f);
    dwb->getDrawable().setScale(1.f, 2.f);

    auto playerController = xy::Component::create<st::PlayerController>(m_messageBus);

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(dwb);
    entity->addComponent(playerController);
    entity->addCommandCategories(playerID);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    auto playerEnt = m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    auto cam = xy::Component::create<xy::Camera>(m_messageBus, getContext().defaultView);
    auto camControl = xy::Component::create<st::CameraController>(m_messageBus, *playerEnt);
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(camControl);
    entity->setPosition(playerEnt->getPosition());
    auto sceneCam = entity->addComponent(cam);
    m_scene.setActiveCamera(sceneCam);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}