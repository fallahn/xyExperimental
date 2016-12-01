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

#include <ThinkTask.hpp>
#include <MessageIDs.hpp>

#include <xygine/Log.hpp>
#include <xygine/util/Random.hpp>

ThinkTask::ThinkTask(xy::Entity& entity, xy::MessageBus& mb)
    : Task(entity, mb),
    m_time(2.f)
{
    
}

//public
void ThinkTask::onStart()
{
    auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
    msg->id = Message::AnimationEvent::Idle;
}

void ThinkTask::update(float dt)
{
    //check for time out, request next task if expired and mark as complete
    m_time -= dt;
    if(m_time <= 0)
    {
        auto msg = getMessageBus().post<Message::TaskEvent>(Message::NewTask);
        msg->taskName = static_cast<Message::TaskEvent::Name>(xy::Util::Random::value(0, 8));
        setCompleted();
    }

    //TODO skew task decision based on needs - ie make eating more likely when hungry
}