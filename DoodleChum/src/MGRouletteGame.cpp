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
#include <AttributeManager.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Resource.hpp>
#include <xygine/Reports.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/util/Position.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/physics/RigidBody.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace
{
    const float summaryTime = 3.f;
}

RouletteGame::RouletteGame(xy::MessageBus& mb, xy::TextureResource& tr, xy::Scene& scene, AttribManager& am)
    : xy::Component     (mb, this),
    m_scene             (scene),
    m_attribManager     (am),
    m_entity            (nullptr),
    m_currentState      ((am.getIncome() > 50) ? State::PlaceBet : State::GameOver),
    m_chargeTimeout     (xy::Util::Random::value(9.f, 11.f)),
    m_chargeTime        (0.f),
    m_wheelActive       (false),
    m_summaryTime       (0.f),
    m_gameoverTime      (5.f),
    m_gameoverAlpha     (0.f),
    m_wheelValue        (6),
    m_triesLeft         (5),
    m_powerbar          (tr.get("assets/images/minigames/roulette/powerbar.png")),
    m_font              (tr.get("assets/fonts/charset_transparent.png"), { 16.f, 16.f }),
    m_creditSelector    (tr.get("assets/images/minigames/roulette/credit_selector.png")),
    m_chanceSelector    (tr.get("assets/images/minigames/roulette/chance_selector.png"))
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
        else if (data.type == Message::InterfaceEvent::MouseClick)
        {
            if (m_currentState == State::PlaceBet)
            {
                auto point = m_entity->getInverseTransform().transformPoint(data.positionX, data.positionY);
                m_chanceSelector.click(point);
                m_creditSelector.click(point);
            }
        }
    };
    addMessageHandler(mh);

    mh.id = xy::Message::PhysicsMessage;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<xy::Message::PhysicsEvent>();
        if (data.event == xy::Message::PhysicsEvent::BeginContact)
        {
            if (data.contact->getCollisionShapeA()->getUserID() != -1)
            {
                m_wheelValue = data.contact->getCollisionShapeA()->getUserID();
            }
            else if(data.contact->getCollisionShapeB()->getUserID() != -1)
            {
                m_wheelValue = data.contact->getCollisionShapeB()->getUserID();
            }
        }
    };
    addMessageHandler(mh);

    mh.id = Message::Attribute;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::AttribEvent>();
        if (data.action == Message::AttribEvent::GotPaid ||
            data.action == Message::AttribEvent::SpentMoney)
        {
            m_creditText.setString("Credit: " + std::to_string(data.value));
        }
    };
    addMessageHandler(mh);

    m_powerbar.setPosition(0.f, 300.f);
    m_reflection.setTexture(tr.get("assets/images/ui/bob_screen.png"));
    m_reflection.setScale(2.f, 2.f);
    xy::Util::Position::centreOrigin(m_reflection);
    m_reflection.setPosition(0.f, 66.f);

    xy::Util::Position::centreOrigin(m_chanceSelector);
    m_chanceSelector.setPosition(-280.f, 270.f);

    xy::Util::Position::centreOrigin(m_creditSelector);
    m_creditSelector.setPosition(280.f, 270.f);

    m_gameOverText.setFont(m_font);
    m_gameOverText.setColour(sf::Color::Black);
    m_gameOverText.setString("GAME OVER");
    m_gameOverText.setScale(5.f, 5.f);
    m_gameOverText.setPosition(0.f, 100.f);
    xy::Util::Position::centreOrigin(m_gameOverText);

    m_summaryText = m_gameOverText;

    m_pressSpaceText.setFont(m_font);
    m_pressSpaceText.setString("Press Space");
    m_pressSpaceText.setColour(sf::Color::Black);
    m_pressSpaceText.setScale(2.f, 2.f);
    xy::Util::Position::centreOrigin(m_pressSpaceText);
    m_pressSpaceText.setPosition(0.f, 330.f);

    m_triesText.setFont(m_font);
    m_triesText.setString("Tries Left: 5");
    m_triesText.setColour(sf::Color::Black);
    m_triesText.setPosition(190.f, -240.f);
    
    m_creditText = m_triesText;
    m_creditText.setString("Credit: " + std::to_string(am.getIncome()));
    m_creditText.move(-600.f, 0.f);

    m_quitTip.setTexture(tr.get("assets/images/ui/quit_tip.png"));
    m_quitTip.setPosition(-100.f, 420.f);
    m_quitTip.setColor(sf::Color::Transparent);
}

//public
void RouletteGame::entityUpdate(xy::Entity& entity, float dt)
{
    static float flashTime = 0.5f;
    auto flash = [&]()
    {
        flashTime -= dt;
        if (flashTime < 0)
        {
            flashTime = 0.5f;
            auto colour = m_pressSpaceText.getColour();
            colour.a = (colour.a == 255) ? 0 : 255;
            m_pressSpaceText.setColour(colour);
        }
    };

    switch (m_currentState)
    {
    default: break;
    case State::PlaceBet:
    {
        flash();

        m_chanceSelector.update(dt);
        m_creditSelector.update(dt);
    }
        break;
    case State::Charging: //moving power bar - -60 to 60, min 30
    {
        m_chargeTime += dt;
        if (m_chargeTime > m_chargeTimeout)
        {
            startWheel();
        }

        m_powerbar.update(dt);
        flash();
    }
        break;
    case State::Running:
    {
        xy::Command cmd;
        cmd.category = Command::ID::RouletteBall;
        cmd.action = [this](xy::Entity& entity, float)
        {
            m_wheelActive = xy::Util::Vector::lengthSquared(entity.getComponent<xy::Physics::RigidBody>()->getLinearVelocity()) > 1.5f;
            //m_wheelActive = entity.getComponent<xy::Physics::RigidBody>()->awake();
        };
        m_scene.sendCommand(cmd);

        if (!m_wheelActive)
        {
            //wheel has stopped, award points
            if (m_wheelValue == 6)
            {
                //std::cout << "Lucky 7!" << std::endl;
                if (m_chanceSelector.getIndex() == 2)
                {
                    win();
                }
                else
                {
                    lose();
                }
            }
            else if (m_wheelValue % 2 == 0)
            {
                //std::cout << "Evens!" << std::endl;
                if (m_chanceSelector.getIndex() == 0)
                {
                    win();
                }
                else
                {
                    lose();
                }
            }
            else
            {
                //std::cout << "Odds!" << std::endl;
                if (m_chanceSelector.getIndex() == 1)
                {
                    win();
                }
                else
                {
                    lose();
                }
            }

            m_triesLeft--;
            m_triesText.setString("Tries Left: " + std::to_string(m_triesLeft));
            
            m_currentState = State::Summary;
            m_summaryTime = summaryTime;          
        }
    }
        break;
    case State::Summary:
    {
        m_summaryTime -= dt;
        if (m_summaryTime < 0)
        {
            if (m_triesLeft > 0 && m_attribManager.getIncome() > 50)
            {
                m_currentState = State::PlaceBet;
                m_powerbar.reset();
            }
            else
            {
                m_currentState = State::GameOver;
            }
        }
    }
        break;
    case State::GameOver:
    {
        m_gameoverTime -= dt;
        if (m_gameoverTime < 0)
        {
            m_gameoverAlpha = std::min(1.f, m_gameoverAlpha + dt);
            sf::Color c(255, 255, 255, static_cast<sf::Uint8>(m_gameoverAlpha * 255.f));
            m_quitTip.setColor(c);
        }
    }
        break;
    }
}


//private
void RouletteGame::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{   
    states.shader = getActiveShader();
    rt.draw(m_powerbar, states);
    rt.draw(m_chanceSelector, states);
    rt.draw(m_creditSelector, states);
    states.shader = nullptr;
    rt.draw(m_triesText, states);
    rt.draw(m_creditText, states);

    switch (m_currentState)
    {
    default: break;
    case State::Charging:
    case State::PlaceBet:
        rt.draw(m_pressSpaceText, states);
        break;
    case State::Summary:
        rt.draw(m_summaryText, states);
        break;
    case State::GameOver:
        rt.draw(m_gameOverText, states);
        rt.draw(m_quitTip, states);
        break;
    }
 
    rt.draw(m_reflection, states);
}

namespace
{
    const float minPower = 30.f;
    const float maxPower = 60.f;
}

void RouletteGame::startWheel()
{
    //read values for input bar
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

void RouletteGame::win()
{
    m_summaryText.setString("WIN");
    xy::Util::Position::centreOrigin(m_summaryText);

    int prize = 0;
    switch (m_creditSelector.getIndex())
    {
    default:
    case 0:
        prize = 10;
        break;
    case 1:
        prize = 20;
        break;
    case 2:
        prize = 50;
        break;
    }

    if (m_chanceSelector.getIndex() == 2)
    {
        prize *= 4;
    }

    m_attribManager.earn(prize);
}

void RouletteGame::lose()
{
    m_summaryText.setString("LOSE");
    xy::Util::Position::centreOrigin(m_summaryText);

    int prize = 0;
    switch (m_creditSelector.getIndex())
    {
    default:
    case 0:
        prize = 10;
        break;
    case 1:
        prize = 20;
        break;
    case 2:
        prize = 50;
        break;
    }

    m_attribManager.spend(prize);
}