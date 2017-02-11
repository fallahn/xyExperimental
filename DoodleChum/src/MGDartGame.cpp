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

#include <MGDarts.hpp>
#include <AttributeManager.hpp>
#include <MessageIDs.hpp>

#include <xygine/Resource.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/App.hpp>
#include <xygine/util/Position.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/components/AudioSource.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Keyboard.hpp>

DartsGame::DartsGame(xy::MessageBus& mb, xy::TextureResource& tr, AttribManager& am)
    : xy::Component (mb, this),
    m_attribManager (am),
    m_entity        (nullptr),
    m_currentState  (State::PlaceBet),
    m_flashColour   (160,160,160),
    m_powerbar      (tr.get("assets/images/minigames/roulette/powerbar.png")),
    m_creditSelector(tr.get("assets/images/minigames/roulette/credit_selector.png")),
    m_wheel         (tr),
    m_dartboard     (tr.get("assets/images/minigames/darts/board.png"), tr.get("assets/images/minigames/darts/dart.png"), mb),
    m_reflection    (tr.get("assets/images/ui/bob_screen.png")),
    m_chargeTimeout (10.f),
    m_chargeTime    (0.f),
    m_pauseTime     (6.f),
    m_gameoverAlpha (0.f),
    m_gameoverTime  (5.f),
    m_tries         (3),
    m_font          (tr.get("assets/fonts/charset_transparent.png"), { 16.f, 16.f }),
    m_target        (0)
{
    m_powerbar.setPosition(0.f, 330.f);

    m_reflection.setScale(2.f, 2.f);
    xy::Util::Position::centreOrigin(m_reflection);
    m_reflection.setPosition(0.f, 66.f);

    m_wheel.setPosition(-300.f, 220.f);

    xy::Util::Position::centreOrigin(m_creditSelector);
    m_creditSelector.setPosition(300.f, 240.f);

    m_messageText.setFont(m_font);
    m_messageText.setString("Press Space");
    m_messageText.setColour(sf::Color::Black);
    m_messageText.setScale(2.f, 2.f);
    xy::Util::Position::centreOrigin(m_messageText);
    m_messageText.setPosition(0.f, 260.f);

    m_triesText.setFont(m_font);
    m_triesText.setString("Darts Left: 3");
    m_triesText.setColour(sf::Color::Black);
    m_triesText.setPosition(190.f, 340.f);

    m_creditText = m_triesText;
    m_creditText.setString("Credit: " + std::to_string(am.getIncome()));
    m_creditText.move(-600.f, 0.f);

    m_targetText.setColour(sf::Color::Black);
    m_targetText.setFont(m_font);
    m_targetText.setString("Target");
    m_targetText.setPosition(-320.f, -240.f);
    xy::Util::Position::centreOrigin(m_targetText);

    m_targetValueText.setFont(m_font);
    m_targetValueText.setScale(4.f, 4.f);
    m_targetValueText.setColour(sf::Color::Black);
    m_targetValueText.setPosition(-320.f, -208.f);
    m_targetValueText.setString("11");
    xy::Util::Position::centreOrigin(m_targetValueText);

    m_scoreText = m_targetText;
    m_scoreText.setString("Score");
    xy::Util::Position::centreOrigin(m_scoreText);
    m_scoreText.move(640.f, 0.f);

    m_scoreValueText = m_targetValueText;
    m_scoreValueText.setString("0");
    xy::Util::Position::centreOrigin(m_scoreValueText);
    m_scoreValueText.move(640.f, 0.f);

    m_quitTip.setTexture(tr.get("assets/images/ui/quit_tip.png"));
    m_quitTip.setPosition(-100.f, 420.f);
    m_quitTip.setColor(sf::Color::Transparent);

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

                    m_creditSelector.setColour(sf::Color::White);
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
                m_creditSelector.click(point, getMessageBus());
            }
            else if (m_currentState == State::Shooting)
            {
                m_dartboard.fire();
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
}

//public
void DartsGame::entityUpdate(xy::Entity& entity, float dt)
{
    static float flashTime = 0.5f;
    auto flash = [&]()
    {
        flashTime -= dt;
        if (flashTime < 0)
        {
            flashTime = 0.5f;
            auto colour = m_messageText.getColour();
            colour.a = (colour.a == 255) ? 0 : 255;
            m_messageText.setColour(colour);

            m_flashColour = (m_flashColour.r == 255) ? sf::Color(160, 160, 160) : sf::Color::White;
        }
    };

    switch (m_currentState)
    {
    default: break;
    case State::PlaceBet:
        m_creditSelector.update(dt);
        m_creditSelector.setColour(m_flashColour);
        flash();
        break;
    case State::Charging:
        m_chargeTime += dt;
        if (m_chargeTime > m_chargeTimeout)
        {
            startWheel();
        }

        m_powerbar.update(dt);
        flash();
        break;
    case State::Spinning:
        m_wheel.update(dt);
        {
            //set string from index
            m_target = m_wheel.getIndex(10) + 11;
            m_targetValueText.setString(std::to_string(m_target));

            auto speed = std::abs(m_wheel.getSpeed());
            xy::Command cmd;
            cmd.category = Command::ID::MiniGame;
            cmd.action = [speed](xy::Entity& entity, float)
            {
                entity.getComponent<xy::AudioSource>()->setPitch((speed / 360.f) + 0.1f);
            };
            m_entity->getScene()->sendCommand(cmd);

            if (m_wheel.stopped())
            {
                m_currentState = State::Shooting;
                m_dartboard.showCrosshair(true);

                m_messageText.setString("Mouse to Throw");
                xy::Util::Position::centreOrigin(m_messageText);

                xy::App::setMouseCursorVisible(false);

                cmd.action = [](xy::Entity& entity, float)
                {
                    entity.getComponent<xy::AudioSource>()->stop();
                };
                m_entity->getScene()->sendCommand(cmd);
            }
        }
        break;
    case State::Shooting:
        m_dartboard.update(dt, entity.getInverseTransform().transformPoint(xy::App::getMouseWorldPosition()));
        
        m_triesText.setString("Darts left: " + std::to_string(m_dartboard.getDartsRemaining()));
        m_scoreValueText.setString(std::to_string(m_dartboard.getScore()));
        xy::Util::Position::centreOrigin(m_scoreValueText);

        flash();
        if (m_dartboard.getScore() > m_target || m_dartboard.getDartsRemaining() == 0)
        {
            xy::App::setMouseCursorVisible(true);
            m_currentState = State::Summary;
            m_dartboard.showCrosshair(false);

            //decide if we won or lost
            auto score = m_dartboard.getScore();
            if (score > m_target || score == 0)
            {
                m_messageText.setString("BUST");
                m_messageText.setColour(sf::Color::Black);
                xy::Util::Position::centreOrigin(m_messageText);
                int cost = (m_creditSelector.getIndex() == 0) ? 10 : (m_creditSelector.getIndex() == 1) ? 20 : 50;
                m_attribManager.spend(cost);
            }
            else
            {
                m_messageText.setString("WIN");
                m_messageText.setColour(sf::Color::Black);
                xy::Util::Position::centreOrigin(m_messageText);
                
                //prize is a percentage of the wager as a ratio of the score to target
                float ratio = static_cast<float>(score) / static_cast<float>(m_target);
                float cost = (m_creditSelector.getIndex() == 0) ? 10.f : (m_creditSelector.getIndex() == 1) ? 20.f : 50.f;
                cost *= ratio;
                m_attribManager.earn(static_cast<std::int32_t>(cost));
            }
        }
        break;
    case State::Summary:
        //count down for some time
    {
        m_pauseTime -= dt;
        if (m_pauseTime < 0)
        {
            if (--m_tries > 0)
            {
                //reset
                m_powerbar.reset();
                m_dartboard.reset();
                m_currentState = State::PlaceBet;
                m_pauseTime = 6.f;
                m_messageText.setString("Press Space");
            }
            else
            {
                m_currentState = State::GameOver;
                m_messageText.setString("GAME OVER");
            }
            xy::Util::Position::centreOrigin(m_messageText);
        }
    }
        break;
    case State::GameOver:
        m_gameoverTime -= dt;
        if (m_gameoverTime < 0)
        {
            m_gameoverAlpha = std::min(1.f, m_gameoverAlpha + dt);
            sf::Color c(255, 255, 255, static_cast<sf::Uint8>(m_gameoverAlpha * 255.f));
            m_quitTip.setColor(c);
        }
        break;
    }
}

//private
void DartsGame::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_powerbar, states);
    rt.draw(m_creditSelector, states);
    rt.draw(m_wheel, states);
    rt.draw(m_dartboard, states);
    rt.draw(m_triesText, states);
    rt.draw(m_creditText, states);

    rt.draw(m_targetText, states);
    rt.draw(m_targetValueText, states);
    rt.draw(m_scoreText, states);
    rt.draw(m_scoreValueText, states);

    switch (m_currentState)
    {
    default:break;
    case State::Charging:
    case State::PlaceBet:
    case State::Shooting:
        rt.draw(m_messageText, states);
        break;
    case State::Summary:
        rt.draw(m_messageText, states);
        break;
    case State::GameOver:
        rt.draw(m_messageText, states);
        rt.draw(m_quitTip, states);
        break;
    }

    rt.draw(m_reflection, states);
}

namespace
{
    const float maxSpeed = 2000.f;
    const float minSpeed = 720.f;
}

void DartsGame::startWheel()
{
    m_currentState = State::Spinning;

    float speed = m_powerbar.getValue() * (maxSpeed - minSpeed);
    if (speed < 0) speed -= minSpeed;
    else speed += minSpeed;

    m_wheel.spin(speed);

    xy::Command cmd;
    cmd.category = Command::ID::MiniGame;
    cmd.action = [](xy::Entity& entity, float)
    {
        entity.getComponent<xy::AudioSource>()->play(true);
    };
    m_entity->getScene()->sendCommand(cmd);
}