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

#include <TravelTask.hpp>
#include <BudController.hpp>
#include <MessageIDs.hpp>

#include <xygine/util/Vector.hpp>
#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Reports.hpp>

namespace
{
    const float minDistance = 100.f;
    const float moveSpeedX = 80.f;
    const float moveSpeedY = 50.f;
}

TravelTask::TravelTask(xy::Entity& entity, xy::MessageBus& mb, std::vector<sf::Vector2f>& points)
    : Task              (entity, mb),
    m_points            (std::move(points)),
    m_currentAnimation  (Message::AnimationEvent::Idle),
    m_moveSpeed         (moveSpeedX),
    m_startDirection    (m_points.back() - getEntity().getWorldPosition())
{

}

//public
void TravelTask::onStart()
{
    setAnimation(m_startDirection);
}

void TravelTask::update(float dt)
{
    auto direction = m_points.back() - getEntity().getWorldPosition();
    auto distance = xy::Util::Vector::lengthSquared(direction);

    if (distance < minDistance)
    {
        m_points.pop_back();
        if (m_points.empty())
        {
            auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
            msg->id = Message::AnimationEvent::Idle;
            setCompleted(Message::TaskEvent::Travel);
        }
        else
        {
            setAnimation(m_points.back() - getEntity().getWorldPosition());
        }
    }
    else //if(distance != 0)
    {
        getEntity().move(xy::Util::Vector::normalise(direction) * m_moveSpeed * dt);

        //broadcast movement
        auto newPos = getEntity().getWorldPosition();
        auto msg = getMessageBus().post<Message::PlayerEvent>(Message::Player);
        msg->action = Message::PlayerEvent::Moved;
        msg->posX = newPos.x;
        msg->posY = newPos.y;
    }
}

//private
void TravelTask::setAnimation(const sf::Vector2f& direction)
{
    float angle = xy::Util::Vector::rotation(direction);
    //REPORT("angle", std::to_string(angle));

    Message::AnimationEvent::ID anim = Message::AnimationEvent::Idle;
    if (angle > -80 && angle < 80) anim = Message::AnimationEvent::Right;
    else if (angle > 81 && angle < 100) anim = Message::AnimationEvent::Down;
    else if (angle > -120 && angle < -80) anim = Message::AnimationEvent::Up;
    else anim = Message::AnimationEvent::Left;

    if (anim != m_currentAnimation)
    {
        auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
        msg->id = anim;

        m_currentAnimation = anim;

        if (anim == Message::AnimationEvent::Left || anim == Message::AnimationEvent::Right)
        {
            m_moveSpeed = moveSpeedX;
        }
        else
        {
            m_moveSpeed = moveSpeedY;
        }
    }
}