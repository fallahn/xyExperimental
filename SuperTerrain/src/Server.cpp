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

#include <Server.hpp>
#include <PacketIDs.hpp>

#include <PlayerController.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Reports.hpp>

using namespace std::placeholders;

namespace
{
    const float snapshotInterval = 1.f / 20.f;
}

Server::Server()
    : m_scene(m_messageBus),
    m_connection(m_messageBus),
    m_snapshotAccumulator(0.f)
{
    m_packetHandler = std::bind(&Server::handlePacket, this, _1, _2, _3, _4, _5);
    m_connection.setPacketHandler(m_packetHandler);
    m_connection.setMaxClients(4);
}

//public
bool Server::start()
{
    m_snapshotAccumulator = 0.f;
    LOG("Server starting...", xy::Logger::Type::Info);
    return m_connection.start();
}

void Server::stop()
{
    m_connection.stop();
}

void Server::update(float dt)
{
    if (m_connection.running())
    {
        while (!m_messageBus.empty())
        {
            const auto& msg = m_messageBus.poll();
            handleMessage(msg);
            m_scene.handleMessage(msg);
        }

        {
            sf::Lock lock(m_connection.getMutex());
            m_scene.update(dt);
        }
        m_connection.update(dt);

        m_snapshotAccumulator += m_snapshotClock.restart().asSeconds();
        while (m_snapshotAccumulator >= snapshotInterval)
        {
            m_snapshotAccumulator -= snapshotInterval;
            sendSnapshot();
        }
#ifdef _DEBUG_
        for (const auto& p : m_players)
        {
            REPORT("Player Server Position", std::to_string(p.worldPosition.x) + ", " + std::to_string(p.worldPosition.y));
        }
#endif //_DEBUG_

    }
}

//private
void Server::spawnPlayer(Player& player)
{
    auto controller = xy::Component::create<st::PlayerController>(m_messageBus);
    
    //TODO check if player has joined before and place at last position, else spawn at centre of world
    sf::Vector2f playerPosition;
    auto playerEntity = xy::Entity::create(m_messageBus);
    playerEntity->setPosition(playerPosition);
    playerEntity->addComponent(controller);
    player.entID = playerEntity->getUID();    
    player.active = true;
    m_scene.addEntity(playerEntity, xy::Scene::Layer::BackRear);

    //let all clients know a new player connected
    sf::Packet packet;
    packet << xy::PacketID(PacketID::PlayerSpawned);
    packet << player.ID << player.entID;
    packet << playerPosition.x << playerPosition.y << player.name;
    m_connection.broadcast(packet, true);

    m_players.push_back(player);
}

void Server::handleMessage(const xy::Message& msg)
{

}

void Server::sendSnapshot()
{
    const auto& ents = m_scene.getLayer(xy::Scene::Layer::BackRear).getChildren();

    sf::Packet packet;
    packet << xy::PacketID(PacketID::PositionUpdate);
    packet << sf::Uint8(ents.size());

    for (const auto& e : ents)
    {
        auto position = e->getPosition();
        packet << e->getUID() << position.x << position.y;
    }
    m_connection.broadcast(packet);

    //send player specific info for reconciliation
    packet.clear();
    packet << xy::PacketID(PacketID::PlayerUpdate) << sf::Uint8(m_players.size());
    for (const auto& p : m_players)
    {
        packet << p.entID << p.worldPosition.x << p.worldPosition.y << p.lastInputID;
    }
    m_connection.broadcast(packet);
}

void Server::handlePacket(const sf::IpAddress& ipAddress, xy::PortNumber portNumber,
    xy::Network::PacketType packetType, sf::Packet& packet, xy::Network::ServerConnection* connection)
{
    switch (packetType)
    {
    default: break;
    case PacketID::PlayerDetails:
    {
        Player player;
        packet >> player.ID >> player.name;        
        
        sf::Lock(m_connection.getMutex());
        if (std::find_if(std::begin(m_players), std::end(m_players),
            [&player](const Player& p) 
        {
            return (p.ID == player.ID);
        }) == m_players.end())
        {
            //check existing active players and notify new client of their existence
            for (const auto& pl : m_players)
            {
                if (pl.active)
                {
                    //create packet with player info and send to just new client
                    sf::Packet pckt;
                    pckt << xy::PacketID(PacketID::PlayerSpawned);
                    pckt << pl.ID << pl.entID;
                    pckt << pl.worldPosition.x << pl.worldPosition.y << pl.name;
                    m_connection.send(player.ID, pckt, true);
                }
            }
            spawnPlayer(player);
        }
    }
    break;
    case PacketID::PlayerInput:
    {
        PlayerInput input;
        packet >> input;
        //TODO best way to map client id to entity ID ?
        auto result = std::find_if(std::begin(m_players), std::end(m_players), [&input](const Player& p)
        {
            return p.ID == input.clientID;
        });
        if (result != m_players.end())
        {
            xy::Command cmd;
            cmd.entityID = result->entID;
            cmd.action = [result, input](xy::Entity& entity, float)
            {
                auto controller = entity.getComponent<st::PlayerController>();
                controller->setInput(input, false);

                result->lastInputID = controller->getLastInputID();
                result->worldPosition = controller->getLastPosition();
            };
            sf::Lock(m_connection.getMutex());
            m_scene.sendCommand(cmd);
        }
    }
        break;
    }
}