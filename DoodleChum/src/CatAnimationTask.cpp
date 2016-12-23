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

#include <CatAnimationTask.hpp>

#include <xygine/util/Random.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/ParticleSystem.hpp>

namespace
{
    const float minSleep = 120.f;
    const float maxSleep = 240.f;
    const float minSit = 60.f;
    const float maxSit = 180.f;
    //const float poop = 6.f;
    const float eat = 10.f;
}

CatAnim::CatAnim(xy::Entity& e, xy::MessageBus& mb, Action action)
    :Task(e, mb),
    m_action(action),
    m_time(10.f)
{

}

//public
void CatAnim::onStart()
{
    auto anim = static_cast<Message::AnimationEvent::ID>(Message::AnimationEvent::Idle | 0xF0);

    switch (m_action)
    {
    default:
    case Action::Eat:
        m_time = eat;
        anim = static_cast<Message::AnimationEvent::ID>(Message::AnimationEvent::Eat | 0xF0);
        break;
    //case Action::Poop:
    //    m_time = poop;
    //    break;
    case Action::Sit:
        m_time = xy::Util::Random::value(minSit, maxSit);
        break;
    case Action::Sleep:
        m_time = xy::Util::Random::value(minSleep, maxSleep);
        getEntity().getComponent<xy::ParticleSystem>()->start();
        break;
    }

    auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
    msg->id = anim;
}

void CatAnim::update(float dt)
{
    int oldTime = static_cast<int>(m_time);
    
    m_time -= dt;
    if (m_time < 0)
    {
        setCompleted(Message::TaskEvent::CatTask);
        if (m_action == Action::Sleep)
        {
            //stop particles
            getEntity().getComponent<xy::ParticleSystem>()->stop();
        }
    }

    else if (m_action != Action::Eat)
    {
        //occasionally play idle anim to flick tail
        if (oldTime > static_cast<int>(m_time) &&
            oldTime % xy::Util::Random::value(10, 14) == 0)
        {
            auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
            msg->id = static_cast<Message::AnimationEvent::ID>(Message::AnimationEvent::Idle | 0xF0);
        }
    }
}