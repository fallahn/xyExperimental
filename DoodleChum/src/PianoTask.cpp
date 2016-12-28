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

#include <PianoTask.hpp>
#include <MessageIDs.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Command.hpp>
#include <xygine/components/ParticleSystem.hpp>
#include <xygine/components/AudioSource.hpp>
#include <xygine/util/Random.hpp>

namespace 
{
    const sf::Vector2f offset(50.f, -100.f);
}

PianoTask::PianoTask(xy::Entity& e, xy::MessageBus& mb, const sf::Vector2f& position)
    : Task      (e, mb),
    m_time      (48.f),
    m_position  (position + offset)
{

}

//public
void PianoTask::onStart()
{
    xy::Command cmd;
    cmd.category = Particle::Music;
    cmd.action = [this](xy::Entity& entity, float dt)
    {
        entity.setWorldPosition(m_position);
        entity.getComponent<xy::ParticleSystem>()->start();
    };
    getEntity().getScene()->sendCommand(cmd);

    //plays a random piano track
    cmd.category = Command::PianoPlayer;
    cmd.action = [this](xy::Entity& entity, float)
    {
        auto musics = entity.getComponents<xy::AudioSource>();
        if (musics.size() == 1)
        {
            musics[0]->play();
            m_time = musics[0]->getDuration();
        }
        else
        {
            auto idx = xy::Util::Random::value(0, musics.size() - 1);
            musics[idx]->play();
            m_time = musics[idx]->getDuration();
        }
        m_time = std::min(m_time, 180.f); //limit to 3 minutes
    };
    getEntity().getScene()->sendCommand(cmd);

    //play anim
    auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
    msg->id = Message::AnimationEvent::Piano;
}

void PianoTask::update(float dt)
{
    m_time -= dt;
    if (m_time <= 0)
    {
        setCompleted(Message::TaskEvent::PlayPiano);

        xy::Command cmd;
        cmd.category = Particle::Music;
        cmd.action = [this](xy::Entity& entity, float dt)
        {
            entity.getComponent<xy::ParticleSystem>()->stop();
        };
        getEntity().getScene()->sendCommand(cmd);
    }
}