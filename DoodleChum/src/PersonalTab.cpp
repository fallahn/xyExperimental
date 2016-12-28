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

#include <PersonalTab.hpp>
#include <AttributeManager.hpp>
#include <MessageIDs.hpp>

#include <xygine/Resource.hpp>
#include <xygine/util/Position.hpp>
#include <xygine/util/Random.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <array>

namespace
{
#include "StringConsts.inl"
#include "BobStrings.inl"

    const sf::Vector2f barOffset(40.f, 220.f);
    const sf::Vector2f barSize(260.f, 45.f);
    const float verticalSpacing = barSize.y + 22.f;
}

PersonalTab::PersonalTab(xy::MessageBus& mb, xy::FontResource& fr, xy::TextureResource& tr, const AttribManager& am)
    : xy::Component(mb, this),
    m_attribManager(am)
{
    auto& font = fr.get("assets/fonts/FallahnHand.ttf");

    m_titleText.setCharacterSize(36u);
    m_titleText.setFillColor(sf::Color::Black);
    m_titleText.setString("Personal");
    m_titleText.setFont(font);
    xy::Util::Position::centreOrigin(m_titleText);
    m_titleText.rotate(-87.f);
    m_titleText.setPosition(466.f, xy::DefaultSceneSize.y / 2.f);

    auto& texture = tr.get("assets/images/ui/value_bar.png");
    const auto values = am.getPersonalAttribs();
    auto position = barOffset;
    for (const auto& a : values)
    {
        m_bars.emplace_back(std::make_unique<ValueBar>(font, texture, barSize));
        m_bars.back()->setPosition(position);
        m_bars.back()->setValue((a.first == AttribManager::Personal::Cleanliness) ? 100.f - a.second : a.second);
        m_bars.back()->setTitle(personalNames[a.first]);

        position.y += verticalSpacing;
    }

    auto& printFont = fr.get("assets/fonts/Printer.ttf");
    m_printout = std::make_unique<Printout>(printFont, tr, mb);
    m_printout->setPosition(40.f, 720.f);

    //message handler for low resources
    xy::Component::MessageHandler mh;
    mh.id = Message::Player;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::PlayerEvent>();
        if (data.action == Message::PlayerEvent::ResourceLow)
        {
            //Household enum
            if (std::find(std::begin(m_messageIDs), std::end(m_messageIDs), data.task) == m_messageIDs.end())
            {
                //TODO these should just look up messages, not copy them
                switch (data.task)
                {
                default: break;
                case AttribManager::Household::Films:
                    m_messageList.push_back(&filmMessages[xy::Util::Random::value(0, filmMessages.size() -1)]);
                    break;
                case AttribManager::Household::Music:
                    m_messageList.push_back(&musicMessages[xy::Util::Random::value(0, musicMessages.size() - 1)]);
                    break;
                case AttribManager::Household::SheetMusic:
                    m_messageList.push_back(&pianoMessages[xy::Util::Random::value(0, pianoMessages.size() - 1)]);
                    break;
                case AttribManager::Household::Games:
                    m_messageList.push_back(&gameMessages[xy::Util::Random::value(0, gameMessages.size() - 1)]);
                    break;
                }
                m_messageIDs.emplace_back(data.task);
            }
        }
        else if (data.action == Message::PlayerEvent::TaskFailed)
        {
            //personal enum
            static const std::int32_t IDOffset = 20;
            if (std::find(std::begin(m_messageIDs), std::end(m_messageIDs), IDOffset + data.task) == m_messageIDs.end())
            {
                switch (data.task)
                {
                default: break;
                case AttribManager::Personal::Cleanliness:
                    m_messageList.push_back(&failedMessages[0]);
                    break;
                case AttribManager::Personal::Hunger:
                    m_messageList.push_back(&failedMessages[1]);
                    break;
                case AttribManager::Personal::Poopiness:
                    m_messageList.push_back(&failedMessages[2]);
                    break;
                case AttribManager::Personal::Thirst:
                    m_messageList.push_back(&failedMessages[3]);
                    break;
                }
                m_messageIDs.emplace_back(IDOffset + data.task);
            }
        }
    };
    addMessageHandler(mh);

    //print all messages (if any) when using computer
    mh.id = Message::Animation;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::AnimationEvent>();
        if (data.id == Message::AnimationEvent::Computer)
        {
            m_printout->clear();

            if (m_messageList.empty())
            {
                m_printout->printLine(randomMessages[xy::Util::Random::value(0, randomMessages.size() - 1)]);
            }

            for (const auto& str : m_messageList)
            {
                m_printout->printLine(*str);
            }
            m_messageList.clear();
            m_messageIDs.clear();
        }
    };
    addMessageHandler(mh);


#ifdef _DEBUG_
    xy::Console::addCommand("printer_print", [this](const std::string& str)
    {
        m_printout->printLine(str);
    });

    xy::Console::addCommand("printer_clear", [this](const std::string&)
    {
        m_printout->clear();
    });

#endif
}

//public
void PersonalTab::entityUpdate(xy::Entity&, float dt)
{
    static float timer = 1.f;

    timer -= dt;
    if (timer <= 0)
    {
        const auto values = m_attribManager.getPersonalAttribs();
        for (auto i = 0u; i < m_bars.size(); ++i)
        {
            auto value = values[i].second;
            //display of cleanliness is inverse
            if (i == AttribManager::Personal::Cleanliness)
            {
                value = 100.f - value;
            }
            m_bars[i]->setValue(value);
        }
        timer = 1.f;
    }

    for (auto& b : m_bars)
    {
        b->update(dt);
    }
    m_printout->update(dt);
}

//private
void PersonalTab::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_titleText, states);

    for (const auto& bar : m_bars)
    {
        rt.draw(*bar, states);
    }

    rt.draw(*m_printout, states);
}