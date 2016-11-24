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

#ifndef ST_WORLDCLIENT_STATE_HPP_
#define ST_WORLDCLIENT_STATE_HPP_

#include <StateIDs.hpp>
#include <Server.hpp>
#include <PlayerInput.hpp>

#include <xygine/State.hpp>
#include <xygine/Scene.hpp>
#include <xygine/network/ClientConnection.hpp>

class WorldClientState final : public xy::State
{
public:
    WorldClientState(xy::StateStack&, Context);
    ~WorldClientState() = default;

    bool update(float) override;
    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    void draw() override;

    xy::StateID stateID() const override { return States::WorldClient; }

private:
    xy::MessageBus& m_messageBus;
    xy::Scene m_scene;

    Server m_server;
    xy::Network::ClientConnection m_connection;
    xy::Network::ClientConnection::PacketHandler m_packetHandler;
    float m_broadcastAccumulator;
    sf::Clock m_broadcastClock;

    //struct PlayerInfo final
    //{
    //    sf::Uint64 entityID = 0;
    //    xy::ClientID clientID = -1;
    //    //std::string name;
    //};
    //std::vector<PlayerInfo> m_playerInfo;
    PlayerInput m_playerInput;
    sf::Uint64 m_playerEntID;
    void handlePacket(xy::Network::PacketType, sf::Packet&, xy::Network::ClientConnection*);

    void addPlayer(xy::ClientID, sf::Uint64, const sf::Vector2f&);
};

#endif //ST_WORLD_CLIENT_STATE_HPP_