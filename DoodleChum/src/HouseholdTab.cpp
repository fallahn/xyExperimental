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

    const sf::Vector2f barOffset(410.f, 260.f);
    const sf::Vector2f barSize(260.f, 45.f);
    const float verticalSpacing = barSize.y + 22.f;
    const std::string balanceString("Balance Cr: ");
}

HouseholdTab::HouseholdTab(xy::MessageBus& mb, xy::FontResource& fr, xy::TextureResource& tr, const AttribManager& am)
    : xy::Component     (mb, this),
    m_attribManager     (am),
    m_entity            (nullptr),
    m_currentActivity   (tr.get("assets/images/ui/current_task.png"))
{
    //tr.get("assets/images/ui/current_task.png").setSmooth(true);

    auto& font = fr.get("assets/fonts/FallahnHand.ttf");

    m_titleText.setCharacterSize(32u);
    m_titleText.setFillColor(sf::Color::Black);
    m_titleText.setString("Household");
    m_titleText.setFont(font);
    xy::Util::Position::centreOrigin(m_titleText);
    m_titleText.rotate(-91.f);
    m_titleText.setPosition(466.f, xy::DefaultSceneSize.y / 2.f);
    m_titleText.setScale(-1.f, 1.f);

    m_balanceText = m_titleText;
    m_balanceText.setString(balanceString + std::to_string(am.getIncome()));
    m_balanceText.setRotation(-2.f);
    m_balanceText.setPosition(400.f, 960.f);
    m_balanceText.setCharacterSize(52u);
    m_balanceText.setOrigin(0.f, 0.f);

    m_currentActivity.setPosition(290.f, 780.f);
    m_currentActivity.setScale(-1.f, 1.f);
    m_activityBorder.setTexture(tr.get("assets/images/ui/pen_border_small.png"));
    m_activityBorder.setPosition(m_currentActivity.getPosition());
    m_activityBorder.setScale(-1.f, 1.f);

    auto& barTexture = tr.get("assets/images/ui/value_bar.png");
    auto& buttonTexture = tr.get("assets/images/ui/add_button.png");
    auto& tagTexture = tr.get("assets/images/ui/price_tag.png");

    const auto& values = am.getHouseholdAttribs();
    const auto& costs = am.getCosts();

    auto position = barOffset;

    for (auto i = 0u; i < values.size(); ++i)
    {
        m_bars.emplace_back(std::make_unique<ValueBar>(font, barTexture, barSize));
        auto& bar = m_bars.back();
        bar->setPosition(position);
        bar->setValue(values[i]);
        bar->setTitle(householdNames[i]);
        bar->setScale(-1.f, 1.f);

        m_buttons.emplace_back(std::make_pair(sf::Sprite(buttonTexture), PriceTag(font, tagTexture)));
        auto& buttonPair = m_buttons.back();
        buttonPair.first.setPosition(14.f, position.y);

        buttonPair.second.setPosition(position);
        const std::string subStr = (i == AttribManager::Household::Water) ? "Cr refill" : "Cr each";
        buttonPair.second.setText("Cost: " + std::to_string(costs[i]) + subStr);
        buttonPair.second.setScale(-1.f, 1.f);

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
                if (m_buttons[i].first.getGlobalBounds().contains(clickPos))
                {
                    auto buttonMsg = sendMessage<Message::InterfaceEvent>(Message::Interface);
                    buttonMsg->type = Message::InterfaceEvent::ButtonClick;
                    buttonMsg->ID = i;
                    break;
                }
            }
        }
        else if (data.type == Message::InterfaceEvent::MouseMoved)
        {
            auto mousePos = m_entity->getWorldTransform().getInverse().transformPoint(data.positionX, data.positionY);
            for (auto i = 0u; i < m_buttons.size(); ++i)
            {
                if (m_buttons[i].first.getGlobalBounds().contains(mousePos))
                {
                    m_buttons[i].second.setVisible(true);
                    m_buttons[i].second.setPosition(mousePos);
                }
                else
                {
                    m_buttons[i].second.setVisible(false);
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

                if (i == AttribManager::Household::Water && !m_buttons.empty())
                {
                    int cost = static_cast<int>((1.f - (values[i] / 100.f)) * static_cast<float>(m_attribManager.getCosts()[i]));
                    m_buttons[i].second.setText("Cost: " + std::to_string(cost) + "Cr refill");
                }
            }
        }
    };
    addMessageHandler(mh);

    //remove all buttons when player dies
    mh.id = Message::Player;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::PlayerEvent>();
        if (data.action == Message::PlayerEvent::Died)
        {
            m_buttons.clear();
        }
    };
    addMessageHandler(mh);

    //set the current activity
    mh.id = Message::NewTask;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::TaskEvent>();
        //don't display idle tasks
        if (data.taskName >= Message::TaskEvent::Idle) return;

        switch (data.taskName)
        {
        default: 
            m_currentActivity.setIndex(data.taskName);
            break;
        case Message::TaskEvent::Think:
        case Message::TaskEvent::Travel:
            break;
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

            if (i == AttribManager::Household::Water && !m_buttons.empty())
            {
                int cost = static_cast<int>((1.f - (values[i] / 100.f)) * static_cast<float>(m_attribManager.getCosts()[i]));
                m_buttons[i].second.setText("Cost: " + std::to_string(cost) + "Cr refill");
            }
        }
        timer = 1.f;
    }

    for (auto& b : m_bars)
    {
        b->update(dt);
    }
    m_currentActivity.update(dt);
}

//private
void HouseholdTab::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_titleText, states);
    rt.draw(m_balanceText, states);
    rt.draw(m_currentActivity, states);
    rt.draw(m_activityBorder, states);

    for (const auto& b : m_bars)
    {
        rt.draw(*b, states);
    }

    for (const auto& b : m_buttons)
    {
        rt.draw(b.first, states);
        rt.draw(b.second, states);
    }
}