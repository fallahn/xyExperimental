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
#include <xygine/components/Camera.hpp>

using namespace st;

namespace
{
    const float moveSpeed = 300.f;
}

PlayerController::PlayerController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_entity        (nullptr),
    m_lastInputID   (0)
{

}

//public
void PlayerController::entityUpdate(xy::Entity& entity, float dt)
{
    if (!m_inputBuffer.empty())
    {
        m_currentInput = std::move(m_inputBuffer.front());
        m_inputBuffer.pop();
    }
        
    auto direction = sf::Vector2f(m_currentInput.mousePosX, m_currentInput.mousePosY) - entity.getWorldPosition();
    auto angle = xy::Util::Vector::rotation(direction);
    entity.setRotation(angle);

    sf::Vector2f velocity;
    //if (m_input & Forward) velocity.y -= 1.f;
    //if (m_input & Back) velocity.y += 1.f;
    //if (m_input & Left) velocity.x -= 1.f;
    //if (m_input & Right) velocity.x += 1.f;

    if (m_currentInput.flags & Forward) velocity.x += 1.f;
    if (m_currentInput.flags & Back) velocity.x -= 1.f;
    if (m_currentInput.flags & Left) velocity.y += 1.f;
    if (m_currentInput.flags & Right) velocity.y -= 1.f;


    if (xy::Util::Vector::lengthSquared(velocity) > 1)
    {
        velocity = xy::Util::Vector::normalise(velocity);
    }
    
    velocity = xy::Util::Vector::rotate(velocity, angle);
    entity.move(velocity * moveSpeed * dt);

    m_lastInputID = m_currentInput.counter;
    m_lastPosition = entity.getWorldPosition();
}

void PlayerController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

void PlayerController::setInput(const PlayerInput& ip, bool keep)
{
    if (keep) m_reconcileInputs.push_back(ip);
    m_inputBuffer.push(ip);
}

void PlayerController::reconcile(const sf::Vector2f& position, sf::Uint64 inputID)
{
    while (!m_reconcileInputs.empty() &&
        m_reconcileInputs.front().counter <= inputID)
    {
        m_reconcileInputs.pop_front();
    }

    m_entity->setWorldPosition(position);

    //make sure the input buffer is empty before doing updates
    std::queue<PlayerInput> newQueue;
    std::swap(m_inputBuffer, newQueue);

    for (const auto& input : m_reconcileInputs)
    {
        m_currentInput = input;
        entityUpdate(*m_entity, 1.f/60.f); //TODO delta value should be stored as part of input
    }
}