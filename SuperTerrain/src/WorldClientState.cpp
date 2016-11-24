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
#include <ClientEntityController.hpp>
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
    const int playerID = 300; //command category for player ent

    //xy::Camera* playerCamera = nullptr;
}

using namespace std::placeholders;

WorldClientState::WorldClientState(xy::StateStack& stateStack, Context context)
    : State                 (stateStack, context),
    m_messageBus            (context.appInstance.getMessageBus()),
    m_scene                 (m_messageBus),
    m_broadcastAccumulator  (0.f),
    m_playerEntID           (0)
{
    launchLoadingScreen();
    m_scene.setView(context.defaultView);

    //TODO option to not start if connecting to remote server
    m_server.start();
    sf::sleep(sf::seconds(2.f));

    m_packetHandler = std::bind(&WorldClientState::handlePacket, this, _1, _2, _3);
    m_connection.setPacketHandler(m_packetHandler);
    m_connection.setServerInfo({ "127.0.0.1" }, xy::Network::ServerPort);
    m_connection.connect();

    auto tc = xy::Component::create<TerrainComponent>(m_messageBus, getContext().appInstance);
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(tc);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    xy::Console::addCommand("connect", [this](const std::string& address)
    {        
        //TODO we need to validate the address string
        m_connection.disconnect();

        m_server.stop();
        sf::sleep(sf::seconds(1.f));

        m_connection.setServerInfo({ address }, xy::Network::ServerPort);

        if (address == "localhost" || address == "127.0.0.1")
        {
            m_server.start();
            sf::sleep(sf::seconds(1.f));
        }

        if (!m_connection.connect())
        {
            xy::Console::print("Failed to connect to " + address);
        }
    });

    quitLoadingScreen();
}

//public
bool WorldClientState::update(float dt)
{
    sf::Uint16 input = 0;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) input |= st::PlayerController::Left;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) input |= st::PlayerController::Right;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) input |= st::PlayerController::Forward;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) input |= st::PlayerController::Back;
    
    auto mousePos = xy::App::getMouseWorldPosition();

    m_playerInput.flags = input;
    m_playerInput.mousePosX = mousePos.x;
    m_playerInput.mousePosY = mousePos.y;

    const float sendRate = 1.f / m_connection.getSendRate();
    m_broadcastAccumulator += m_broadcastClock.restart().asSeconds();
    while (m_broadcastAccumulator >= sendRate)
    {
        m_playerInput.clientID = m_connection.getClientID();
        m_playerInput.timestamp = m_connection.getTime().asMilliseconds();

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
    cmd.action = [this](xy::Entity& entity, float dt)
    {
        entity.getComponent<st::PlayerController>()->setInput(m_playerInput);
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
        /*xy::ClientID id;
        packet >> id;

        if (id == m_connection.getClientID())*/
        {
            sf::Packet newPacket;
            newPacket << xy::PacketID(PacketID::PlayerDetails) << m_connection.getClientID() << "Player";
            connection->send(newPacket, true);
            //std::cout << "got connected packet" << std::endl;
        }
    }
    break;
    case xy::Network::Disconnect:
    {
        xy::Command cmd;
        cmd.entityID = m_playerEntID;
        cmd.action = [this](xy::Entity& e, float) { e.destroy(); m_playerEntID = 0; };
        sf::Lock lock(m_connection.getMutex());
        m_scene.sendCommand(cmd);

        m_playerInfo.erase(std::remove_if(std::begin(m_playerInfo), std::end(m_playerInfo), 
            [this](const PlayerInfo& pi)
        { 
            return pi.clientID == m_connection.getClientID(); 
        }));
    }
        break;
    case xy::Network::ClientLeft:
    {
        xy::ClientID clid;
        packet >> clid;

        auto result = std::find_if(std::begin(m_playerInfo), std::end(m_playerInfo),
            [clid](const PlayerInfo& pi)
        {
            return pi.clientID == clid;
        });

        if (result != m_playerInfo.end())
        {
            xy::Command cmd;
            cmd.entityID = result->entityID;
            cmd.action = [](xy::Entity& e, float) { e.destroy(); };
            sf::Lock lock(m_connection.getMutex());
            m_scene.sendCommand(cmd);

            m_playerInfo.erase(result);
            xy::Logger::log("Player " + std::to_string(clid) + " has left game", xy::Logger::Type::Info);
        }
    }
    break;
    case PacketID::PlayerSpawned:
    {
        sf::Lock(m_connection.getMutex());
        xy::ClientID clid; sf::Uint64 entid; sf::Vector2f position;
        packet >> clid >> entid >> position.x >> position.y;
        sf::Lock(m_connection.getMutex());
        {
            addPlayer(clid, entid, position);
            xy::Logger::log("Player " + std::to_string(clid) + " has joined game", xy::Logger::Type::Info);
        }
    }
        break;
    case PacketID::PositionUpdate:
        //updates all the non-predicted entities from the server via interpolation
    {
        sf::Uint8 count;
        packet >> count;

        sf::Uint64 entID;
        sf::Vector2f position;

        while (count--)
        {
            packet >> entID >> position.x >> position.y;
            if (entID != m_playerEntID)
            {
                xy::Command cmd;
                cmd.action = [position](xy::Entity& entity, float)
                {
                    entity.getComponent<st::NetworkController>()->setDestination(position);
                };
                cmd.entityID = entID;

                sf::Lock(m_connection.getMutex());
                m_scene.sendCommand(cmd);
            }
        }
    }
        break;
    case PacketID::PlayerUpdate:
        //updates the clientside predicted entities (our local player) with server correction
    {
        sf::Uint8 count;
        packet >> count;

        sf::Uint64 entID;
        sf::Vector2f position;
        sf::Uint64 lastInput;

        while (count--)
        {
            packet >> entID >> position.x >> position.y >> lastInput;
            if (entID == m_playerEntID)
            {
                xy::Command cmd;
                cmd.action = [position, lastInput](xy::Entity& entity, float)
                {
                    entity.getComponent<st::PlayerController>()->reconcile(position, lastInput);
                };
                cmd.entityID = entID;

                sf::Lock(m_connection.getMutex());
                m_scene.sendCommand(cmd);
                break;
            }
        }
    }
        break;
    }
}

void WorldClientState::addPlayer(xy::ClientID clid, sf::Uint64 entid, const sf::Vector2f& position)
{
    auto dwb = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(m_messageBus);
    dwb->getDrawable().setRadius(20.f);
    dwb->getDrawable().setOrigin(20.f, 20.f);
    dwb->getDrawable().setFillColor(sf::Color::Red);
    dwb->getDrawable().setPointCount(3);
    dwb->getDrawable().setRotation(90.f);
    dwb->getDrawable().setScale(1.f, 2.f);
    
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(dwb);
    entity->setPosition(position);
    entity->setUID(entid);
    auto playerEnt = m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    //if this is our player add control and camera
    if (clid == m_connection.getClientID())
    {
        auto playerController = xy::Component::create<st::PlayerController>(m_messageBus);
        playerEnt->addComponent(playerController);
        playerEnt->addCommandCategories(playerID);
        m_playerEntID = entid;

        auto cam = xy::Component::create<xy::Camera>(m_messageBus, getContext().defaultView);
        auto camControl = xy::Component::create<st::CameraController>(m_messageBus, *playerEnt);
        entity = xy::Entity::create(m_messageBus);
        entity->addComponent(camControl);
        entity->setPosition(playerEnt->getPosition());
        auto sceneCam = entity->addComponent(cam);
        m_scene.setActiveCamera(sceneCam);
        m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
    }
    else
    {
        //add a interpolator controller
        auto entController = xy::Component::create<st::NetworkController>(m_messageBus);
        playerEnt->addComponent(entController);
        //TODO could we set the command cat to client ID? Iiiiinteresting...
    }

    //log player details so we can remove them again when  they leave
    m_playerInfo.emplace_back(clid, entid);
}