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

#include <IdleTask.hpp>
#include <MessageIDs.hpp>

#include <xygine/util/Random.hpp>

IdleTask::IdleTask(xy::Entity& e, xy::MessageBus& mb, std::int32_t animID, std::int32_t taskID)
    : Task      (e, mb),
    m_animID    (animID),
    m_taskID    (taskID),
    m_time      (xy::Util::Random::value(2.f, 6.f))
{

}

//public
void IdleTask::onStart()
{
    auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
    if (m_animID != -1 && m_animID < Message::AnimationEvent::Count)
    {       
        msg->id = static_cast<Message::AnimationEvent::ID>(m_animID);

        switch (m_animID)
        {
        default: break;
        case Message::AnimationEvent::Water:
            m_time = 3.f;
            break;
        case Message::AnimationEvent::Feed:
            m_time = 2.f;
            break;
        }
    }
    else
    {
        msg->id = Message::AnimationEvent::Idle;
    }
}

void IdleTask::update(float dt)
{
    //TODO random animation triggers?
    
    m_time -= dt;
    if (m_time <= 0)
    {
        setCompleted(static_cast<Message::TaskEvent::Name>(m_taskID));
    }
}