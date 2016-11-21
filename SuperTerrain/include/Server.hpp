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

#ifndef ST_SERVER_HPP_
#define ST_SERVER_HPP_

#include <xygine/network/ServerConnection.hpp>
#include <xygine/Scene.hpp>
#include <xygine/MessageBus.hpp>

class Server final
{
public:
    Server();
    ~Server() = default;

    bool start();
    void stop();

    void update(float);


private:
    xy::MessageBus m_messageBus;
    xy::Scene m_scene;

    xy::Network::ServerConnection m_connection;
    xy::Network::ServerConnection::PacketHandler m_packetHandler;
    float m_snapshotAccumulator;
    sf::Clock m_snapshotClock;

    struct Player final
    {
        xy::ClientID ID = -1;
        std::string name;
        sf::Uint8 number = 0;
        sf::Uint64 entID = 0;
        sf::Vector2f worldPosition;
        sf::Uint64 lastInputID;
        bool active = false;
    };

    std::vector<Player> m_players;
    void spawnPlayer(Player&);

    void handleMessage(const xy::Message&);
    void sendSnapshot();
    void handlePacket(const sf::IpAddress&, xy::PortNumber, xy::Network::PacketType, sf::Packet&, xy::Network::ServerConnection*);
};


#endif //ST_SERVER_HPP_