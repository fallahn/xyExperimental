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

#include <SleepTask.hpp>
#include <MessageIDs.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Command.hpp>
#include <xygine/components/ParticleSystem.hpp>

namespace
{
    const sf::Vector2f offset(140.f, -110.f);
}

SleepTask::SleepTask(xy::Entity& e, xy::MessageBus& mb, const sf::Vector2f& position)
    : Task(e, mb),
    m_time(10.f),
    m_position(position + offset)
{

}

//public
void SleepTask::onStart()
{
    xy::Command cmd;
    cmd.category = Particle::Sleep;
    cmd.action = [this](xy::Entity& entity, float dt)
    {
        entity.setWorldPosition(m_position);
        entity.getComponent<xy::ParticleSystem>()->start();
    };
    getEntity().getScene()->sendCommand(cmd);

    //make him lie on his back
    getEntity().setRotation(76.f);
    getEntity().move(0.f, -40.f);

    //anim
    auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
    msg->id = Message::AnimationEvent::Sleep;
}

void SleepTask::update(float dt)
{
    m_time -= dt;
    if (m_time <= 0)
    {
        setCompleted(Message::TaskEvent::Sleep);

        xy::Command cmd;
        cmd.category = Particle::Sleep;
        cmd.action = [this](xy::Entity& entity, float dt)
        {
            entity.getComponent<xy::ParticleSystem>()->stop();
        };
        getEntity().getScene()->sendCommand(cmd);

        getEntity().setRotation(0.f);
        getEntity().move(0.f, 40.f);
    }
}
