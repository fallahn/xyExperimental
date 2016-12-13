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

#include <HouseholdTab.hpp>
#include <AttributeManager.hpp>
#include <MessageIDs.hpp>

#include <xygine/util/Position.hpp>
#include <xygine/Resource.hpp>
#include <xygine/Entity.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
#include "StringConsts.inl"

    const sf::Vector2f barOffset(410.f, 220.f);
    const sf::Vector2f barSize(260.f, 45.f);
    const float verticalSpacing = barSize.y + 22.f;
    const std::string balanceString("Balance Cr: ");
}

HouseholdTab::HouseholdTab(xy::MessageBus& mb, xy::FontResource& fr, xy::TextureResource& tr, const AttribManager& am)
    : xy::Component (mb, this),
    m_attribManager (am),
    m_entity        (nullptr)
{
    auto& font = fr.get("assets/fonts/FallahnHand.ttf");

    m_titleText.setCharacterSize(36u);
    m_titleText.setFillColor(sf::Color::Black);
    m_titleText.setString("Household");
    m_titleText.setFont(font);
    xy::Util::Position::centreOrigin(m_titleText);
    m_titleText.rotate(-92.f);
    m_titleText.setPosition(466.f, xy::DefaultSceneSize.y / 2.f);
    m_titleText.setScale(-1.f, 1.f);

    m_balanceText = m_titleText;
    m_balanceText.setString(balanceString + std::to_string(am.getIncome()));
    m_balanceText.setRotation(-2.f);
    m_balanceText.setPosition(400.f, 800.f);
    m_balanceText.setCharacterSize(52u);
    m_balanceText.setOrigin(0.f, 0.f);

    auto& barTexture = tr.get("assets/images/ui/value_bar.png");
    auto& buttonTexture = tr.get("assets/images/ui/add_button.png");
    const auto values = am.getHouseholdAttribs();
    auto position = barOffset;

    for (auto i = 0u; i < values.size(); ++i)
    {
        m_bars.emplace_back(std::make_unique<ValueBar>(font, barTexture, barSize));
        m_bars.back()->setPosition(position);
        m_bars.back()->setValue(values[i]);
        m_bars.back()->setTitle(householdNames[i]);
        m_bars.back()->setScale(-1.f, 1.f);

        m_buttons.emplace_back(buttonTexture);
        m_buttons.back().setPosition(14.f, position.y);

        position.y += verticalSpacing;
    }
    m_buttons.pop_back(); //we don't want one for income rate

    //message handler to check each button on mouse click
    xy::Component::MessageHandler mh;
    mh.id = Message::Interface;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::InterfaceEvent>();
        if (data.type == Message::InterfaceEvent::MouseClick)
        {
            auto clickPos = m_entity->getWorldTransform().getInverse().transformPoint(data.positionX, data.positionY);
            for (auto i = 0u; i < m_buttons.size(); ++i)
            {
                if (m_buttons[i].getGlobalBounds().contains(clickPos))
                {
                    auto buttonMsg = sendMessage<Message::InterfaceEvent>(Message::Interface);
                    buttonMsg->type = Message::InterfaceEvent::ButtonClick;
                    buttonMsg->ID = i;
                    break;
                }
            }
        }
    };
    addMessageHandler(mh);

    //message handler to update balance    
    mh.id = Message::Attribute;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::AttribEvent>();
        if (data.action == Message::AttribEvent::GotPaid
            || data.action == Message::AttribEvent::SpentMoney)
        {
            m_balanceText.setString(balanceString + std::to_string(data.value));
            //so bars update immediately
            const auto values = m_attribManager.getHouseholdAttribs();
            for (auto i = 0u; i < m_bars.size(); ++i)
            {
                m_bars[i]->setValue(values[i]);
            }
        }
    };
    addMessageHandler(mh);
}

//public
void HouseholdTab::entityUpdate(xy::Entity&, float dt)
{
    static float timer = 1.f;

    timer -= dt;
    if (timer <= 0)
    {
        const auto values = m_attribManager.getHouseholdAttribs();
        for (auto i = 0u; i < m_bars.size(); ++i)
        {
            m_bars[i]->setValue(values[i]);
        }
        timer = 1.f;
    }

    for (auto& b : m_bars)
    {
        b->update(dt);
    }
}

//private
void HouseholdTab::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_titleText, states);
    rt.draw(m_balanceText, states);

    for (const auto& b : m_bars)
    {
        rt.draw(*b, states);
    }

    for (const auto& b : m_buttons)
    {
        rt.draw(b, states);
    }
}