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

#include <PlayerController.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Reports.hpp>
#include <xygine/util/Vector.hpp>

using namespace st;

namespace
{
    const float moveSpeed = 300.f;
}

PlayerController::PlayerController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_entity        (nullptr),
    m_input         (0u),
    m_lastInput     (0u)
{

}

//public
void PlayerController::entityUpdate(xy::Entity& entity, float dt)
{
    entity.setRotation(static_cast<sf::Int16>((m_input & 0xffff0000) >> 16));

    sf::Vector2f velocity;
    if (m_input & Forward) velocity.y -= 1.f;
    if (m_input & Back) velocity.y += 1.f;
    if (m_input & Left) velocity.x -= 1.f;
    if (m_input & Right) velocity.x += 1.f;
    if (xy::Util::Vector::lengthSquared(velocity) > 1)
    {
        velocity = xy::Util::Vector::normalise(velocity);
    }
    
    //velocity = xy::Util::Vector::rotate(velocity, entity.getRotation());
    entity.move(velocity * moveSpeed * dt);
}

void PlayerController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}