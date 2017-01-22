/*********************************************************************
Matt Marchant 2017
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

#include <MGRoulette.hpp>
#include <MessageIDs.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Resource.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/util/Position.hpp>
#include <xygine/physics/RigidBody.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Keyboard.hpp>

RouletteGame::RouletteGame(xy::MessageBus& mb, xy::TextureResource& tr, xy::Scene& scene)
    : xy::Component     (mb, this),
    m_textureResource   (tr),
    m_scene             (scene),
    m_currentState      (State::PlaceBet),
    m_chargeTimeout     (xy::Util::Random::value(9.f, 11.f)),
    m_chargeTime        (0.f),
    m_wheelActive       (false),
    m_powerbar          (tr.get("assets/images/minigames/roulette/powerbar.png"))
{
    xy::Component::MessageHandler mh;
    mh.id = Message::Interface;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::InterfaceEvent>();
        if (data.type == Message::InterfaceEvent::KeyPress)
        {
            if (static_cast<sf::Keyboard::Key>(data.ID) == sf::Keyboard::Space)
            {
                if (m_currentState == State::PlaceBet)
                {
                    m_currentState = State::Charging;
                    m_chargeTimeout = xy::Util::Random::value(9.f, 11.f);
                    m_chargeTime = 0.f;
                }
                else if (m_currentState == State::Charging)
                {                    
                    startWheel();
                }
            }
        }
    };
    addMessageHandler(mh);

    m_powerbar.setPosition(0.f, 300.f);
    m_reflection.setTexture(tr.get("assets/images/ui/bob_screen.png"));
    m_reflection.setScale(2.f, 2.f);
    xy::Util::Position::centreOrigin(m_reflection);
    m_reflection.setPosition(0.f, 66.f);
}

//public
void RouletteGame::entityUpdate(xy::Entity& entity, float dt)
{
    switch (m_currentState)
    {
    default: break;
    case State::PlaceBet: //bet amount slider / bet outcome checkbox

        break;
    case State::Charging: //moving power bar - -60 to 60, min 30
    {
        m_chargeTime += dt;
        if (m_chargeTime > m_chargeTimeout)
        {
            startWheel();
        }

        m_powerbar.update(dt);
    }
        break;
    case State::Running:
    {
        xy::Command cmd;
        cmd.category = Command::ID::RouletteBall;
        cmd.action = [this](xy::Entity& entity, float)
        {
            m_wheelActive = entity.getComponent<xy::Physics::RigidBody>()->awake();
        };
        m_scene.sendCommand(cmd);

        if (!m_wheelActive)
        {
            //wheel has stopped, award points

            m_currentState = State::PlaceBet;
            m_powerbar.reset();
        }
    }
        break;
    }
}


//private
void RouletteGame::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_powerbar, states);
    rt.draw(m_reflection, states);
}

namespace
{
    const float minPower = 30.f;
    const float maxPower = 60.f;
}

void RouletteGame::startWheel()
{
    //TODO read values for input bar
    float impulse = m_powerbar.getValue();
    impulse *= (maxPower - minPower);
    if (impulse > 0)
    {
        impulse += minPower;
    }
    else
    {
        impulse -= minPower;
    }

    xy::Command cmd;
    cmd.category = Command::ID::RouletteWheel;
    cmd.action = [impulse](xy::Entity& entity, float)
    {
        entity.getComponent<xy::Physics::RigidBody>()->applyAngularImpulse(impulse); //min amount 30, max 60
    };
    m_scene.sendCommand(cmd);

    cmd.category = Command::ID::RouletteBall;
    cmd.action = [impulse](xy::Entity& entity, float)
    {
        entity.getComponent<xy::Physics::RigidBody>()->applyAngularImpulse(impulse * 40.f);
        entity.getComponent<xy::Physics::RigidBody>()->applyLinearImpulse({ 0.f, -100.f }, {});
    };
    m_scene.sendCommand(cmd);

    m_currentState = State::Running;
    m_wheelActive = true;
}