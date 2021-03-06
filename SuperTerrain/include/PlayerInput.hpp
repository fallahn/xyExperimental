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

#ifndef ST_PLAYER_INPUT_HPP_
#define ST_PLAYER_INPUT_HPP_

#include <SFML/Config.hpp>
#include <xygine/network/Config.hpp>

struct PlayerInput final
{
    sf::Uint16 flags = 0;
    sf::Uint64 counter = 0; //< input counter for ordering
    xy::ClientID clientID = -1;
    sf::Int32 timestamp = 0; //< server time as client knows it
    float mousePosX = 0.f;
    float mousePosY = 0.f;
};

sf::Packet& operator << (sf::Packet& packet, const PlayerInput& input);
sf::Packet& operator >> (sf::Packet& packet, PlayerInput& input);

#endif //ST_PLAYER_INPUT_HPP_