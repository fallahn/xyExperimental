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

#include <VacuumTask.hpp>

#include <xygine/Entity.hpp>

namespace
{
    const float moveSpeed = 80.f;
    const float maxDistance = 1000.f;
}

VacuumTask::VacuumTask(xy::Entity& entity, xy::MessageBus& mb)
    : Task          (entity, mb),
    m_startPosition (),
    m_returning     (false),
    m_moveSpeed     (0.f)
{

}

//public
void VacuumTask::onStart()
{
    auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
    msg->id = Message::AnimationEvent::VacuumStill;

    m_startPosition = getEntity().getWorldPosition();
}

void VacuumTask::update(float dt)
{
    getEntity().move(m_moveSpeed * dt, 0.f);
        
    const auto position = getEntity().getWorldPosition();
    //broadcsat movement
    auto moveMsg = getMessageBus().post<Message::PlayerEvent>(Message::Player);
    moveMsg->action = Message::PlayerEvent::Moved;
    moveMsg->posX = position.x;
    moveMsg->posY = position.y;

    float travelledDistance = position.x - m_startPosition.x;

    if (!m_returning)
    {
        static float elapsed = 0.f;
        elapsed += dt;
        if (elapsed > 2)
        {
            elapsed = 0.f;
            //switch animation
            if (m_moveSpeed == 0)
            {
                m_moveSpeed = moveSpeed;
                auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
                msg->id = Message::AnimationEvent::VacuumWalk;
            }
            else
            {             
                m_moveSpeed = 0.f;
                auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
                msg->id = Message::AnimationEvent::VacuumStill; 
            }
        }

        if (travelledDistance > maxDistance)
        {
            m_returning = true;
            m_moveSpeed = -moveSpeed;

            auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
            msg->id = Message::AnimationEvent::Left;
        }
    }

    if (m_returning &&
      travelledDistance < 6.f)
    {
        m_moveSpeed = 0.f;
        setCompleted(Message::TaskEvent::Vacuum);
        auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
        msg->id = Message::AnimationEvent::Idle;
    }
}