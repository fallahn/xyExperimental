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

#include <TimeTab.hpp>
#include <MessageIDs.hpp>
#include <AttributeManager.hpp>
#include <StateIDs.hpp>

#include <xygine/Resource.hpp>
#include <xygine/Entity.hpp>
#include <xygine/util/Position.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <sstream>

namespace
{
    const std::string balanceString("Balance Cr: ");
    const std::string daysString("Days to pay day:  ");
}

TimeTab::TimeTab(xy::MessageBus& mb, xy::FontResource& fr, xy::TextureResource& tr, const AttribManager& am)
    : xy::Component (mb, this),
    m_attribManager (am),
    m_entity        (nullptr)
{
    auto& handFont = fr.get("assets/fonts/FallahnHand.ttf");

    m_titleText.setFont(handFont);
    m_titleText.setString("Info...");
    m_titleText.setFillColor(sf::Color::Black);
    m_titleText.setCharacterSize(36u);
    m_titleText.setRotation(3.f);
    xy::Util::Position::centreOrigin(m_titleText);
    m_titleText.setPosition(xy::DefaultSceneSize.x / 2.f, 196.f);

    m_daysText.setFont(handFont);
    m_daysText.setString(daysString + std::to_string(m_attribManager.getDaysToPayDay()));
    m_daysText.setCharacterSize(50u);
    m_daysText.setFillColor(sf::Color::Black);
    xy::Util::Position::centreOrigin(m_daysText);
    m_daysText.setPosition(420.f, 80.f);

    m_balanceText = m_daysText;
    m_balanceText.setString(balanceString + std::to_string(am.getIncome()));
    xy::Util::Position::centreOrigin(m_balanceText);
    m_balanceText.move(1100.f, 0.f);

    auto& clockFont = fr.get("assets/fonts/Clock.ttf");
    m_clockText.setFont(clockFont);
    m_clockText.setFillColor({ 255, 0, 0, 140 });
    m_clockText.setCharacterSize(100);
    m_clockText.setString("00:00");
    xy::Util::Position::centreOrigin(m_clockText);
    m_clockText.setPosition(xy::DefaultSceneSize.x / 2.f, 84.f);

    m_clockShadow = m_clockText;
    m_clockShadow.move(1.f, 1.f);
    m_clockShadow.setString("00:00");
    m_clockShadow.setFillColor({ 160, 160, 160, 160 });

    m_buttonSprite.setTexture(tr.get("assets/images/ui/menu_button.png"));
    m_buttonSprite.setPosition(xy::DefaultSceneSize.x - m_buttonSprite.getLocalBounds().width - 10.f, 90.f);

    m_calendarSprite.setTexture(tr.get("assets/images/ui/calendar.png"));
    xy::Util::Position::centreOrigin(m_calendarSprite);
    m_calendarSprite.setPosition(618.f, 80.f);


    xy::Component::MessageHandler mh;
    mh.id = Message::TimeOfDay;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::TODEvent>();
        updateClockText(data.time);
    };
    addMessageHandler(mh);

    mh.id = Message::Attribute;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::AttribEvent>();
        if (data.action == Message::AttribEvent::GotPaid
            || data.action == Message::AttribEvent::SpentMoney)
        {
            m_balanceText.setString(balanceString + std::to_string(data.value));
        }
    };
    addMessageHandler(mh);

    mh.id = Message::DayChanged;
    mh.action = [this](xy::Component*, const xy::Message&)
    {
        m_daysText.setString(daysString + std::to_string(m_attribManager.getDaysToPayDay()));
    };
    addMessageHandler(mh);

    mh.id = Message::Interface;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::InterfaceEvent>();
        if (m_buttonSprite.getGlobalBounds().contains(m_entity->getInverseTransform().transformPoint(data.positionX, data.positionY)))
        {
            auto menuMsg = sendMessage<xy::Message::UIEvent>(xy::Message::UIMessage);
            menuMsg->type = xy::Message::UIEvent::RequestState;
            menuMsg->stateID = States::ID::Menu;
        }
    };
    addMessageHandler(mh);
}

//public
void TimeTab::entityUpdate(xy::Entity&, float)
{

}

//private
void TimeTab::updateClockText(float time)
{
    //somehow we ended up with 00 as midday..
    int minutes = int(time / 60.f);
    int hours = minutes / 60;
    minutes = minutes % 60;

    hours = (hours > 12) ? hours - 12 : hours + 12;

    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << hours % 24 << ":"
        << std::setw(2) << std::setfill('0') << minutes;

    m_clockText.setString(ss.str());
}

void TimeTab::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_calendarSprite, states);
    rt.draw(m_buttonSprite, states);    
    rt.draw(m_titleText, states);
    rt.draw(m_daysText, states);
    rt.draw(m_balanceText, states);
    rt.draw(m_clockShadow, states);
    rt.draw(m_clockText, states);
}