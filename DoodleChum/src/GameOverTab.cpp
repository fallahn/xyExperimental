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

#include <GameOverTab.hpp>
#include <AttributeManager.hpp>
#include <MessageIDs.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Position.hpp>
#include <xygine/Resource.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

GameOverTab::GameOverTab(xy::MessageBus& mb, sf::Font& font, xy::TextureResource& tr, AttribManager& am)
    : xy::Component (mb, this),
    m_entity        (nullptr),
    m_scale         (0.f)
{
    m_backgroundSprite.setTexture(tr.get("assets/images/ui/game_over_tab.png"));
    xy::Util::Position::centreOrigin(m_backgroundSprite);

    m_titleText.setFillColor(sf::Color::Black);
    m_titleText.setFont(font);

    m_dateText = m_titleText;

    m_titleText.setCharacterSize(90u);
    m_titleText.setString("DoodleBob has expired...");
    xy::Util::Position::centreOrigin(m_titleText);
    m_titleText.setPosition(-16.f, -202.f);

    m_dateText.setString(am.getBirthdates());
    m_dateText.setPosition(-130.f, -100.f);
    m_dateText.setCharacterSize(40u);
    xy::Util::Position::centreOrigin(m_dateText);

    m_statText = m_dateText;
    m_statText.setString(am.getIncomeStats());
    xy::Util::Position::centreOrigin(m_statText);
    m_statText.setPosition(0.f, 220.f);

    m_ageText = m_dateText;
    auto age = am.getAge();
    m_ageText.setString("Aged " + std::to_string(age) + " days");
    xy::Util::Position::centreOrigin(m_ageText);
    m_ageText.move(0.f, 46.f);

    m_messageText = m_ageText;
    switch (age)
    {
    case 0:
    case 1:
    case 2:
        m_messageText.setString("Why do the good die young?");
        break;
    case 3:
    case 4:
    case 5:
    case 6:
        m_messageText.setString("He lived longer than my goldfish");
        break;
    case 7:
    case 8:
    case 9:
        m_messageText.setString("At least he made it to pay day");
        break;
    default:
        m_messageText.setString("He lived a good long life");
        break;
    }
    xy::Util::Position::centreOrigin(m_messageText);
    m_messageText.move(0.f, 46.f);

    m_buttonSprite.setTexture(tr.get("assets/images/ui/new_game_button.png"));
    xy::Util::Position::centreOrigin(m_buttonSprite);
    m_buttonSprite.setPosition(0.f, 330.f);

    //call back for new game button
    xy::Component::MessageHandler mh;
    mh.id = Message::Interface;
    mh.action = [this, &am](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::InterfaceEvent>();
        if (data.type == Message::InterfaceEvent::MouseClick)
        {
            auto mousePos = m_entity->getInverseTransform().transformPoint(data.positionX, data.positionY);
            if (m_buttonSprite.getGlobalBounds().contains(mousePos))
            {
                am.reset();
                //request stack clear / new game state
                auto newMsg = sendMessage<Message::SystemEvent>(Message::System);
                newMsg->action = Message::SystemEvent::ResetGame;
            }
        }
    };
    addMessageHandler(mh);
}

//public
void GameOverTab::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_scale < 1)
    {
        entity.setScale(m_scale, m_scale);
        m_scale += dt * 2.f;
    }
}

//private
void GameOverTab::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_backgroundSprite, states);
    rt.draw(m_titleText, states);
    rt.draw(m_dateText, states);
    rt.draw(m_statText, states);
    rt.draw(m_ageText, states);
    rt.draw(m_messageText, states);
    rt.draw(m_buttonSprite, states);
}