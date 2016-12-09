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

#include <TabComponent.hpp>
#include <MessageIDs.hpp>

#include <xygine/Entity.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    const sf::Vector2f tabSize(40.f, 200.f);
    const float moveSpeed = 2500.f;
    const sf::Color vertColour(255, 255, 255, 230);
}

TabComponent::TabComponent(xy::MessageBus& mb, const sf::Vector2f& size, Direction direction)
    : xy::Component     (mb, this),
    m_moving            (false),
    m_travelDistance    (direction == Direction::Horizontal ? size.x : size.y),
    m_distanceRemaining (0.f),
    m_entity            (nullptr)
{
    m_vertices.emplace_back(sf::Vector2f(), vertColour);
    m_vertices.emplace_back(sf::Vector2f(size.x, 0.f), vertColour);
    m_vertices.emplace_back(size, vertColour);
    m_vertices.emplace_back(sf::Vector2f(0.f, size.y), vertColour);

    m_globalBounds.width = size.x;
    m_globalBounds.height = size.y;

    if (direction == Direction::Horizontal)
    {
        m_velocity.x = -1.f; //initial click inverts this
        sf::Vector2f tabOffset = { size.x,  (size.y - tabSize.y) / 2.f };
        float tabDent = tabSize.y / 6.f;

        m_vertices.emplace_back(tabOffset, vertColour);
        m_vertices.emplace_back(sf::Vector2f(tabOffset.x + tabSize.x, tabOffset.y + tabDent), vertColour);
        m_vertices.emplace_back(sf::Vector2f(tabOffset.x + tabSize.x, tabOffset.y + (tabSize.y - tabDent)), vertColour);
        m_vertices.emplace_back(sf::Vector2f(tabOffset.x, tabOffset.y + tabSize.y), vertColour);

        m_globalBounds.width += tabSize.x;
        m_tabBounds = { tabOffset, tabSize };
    }
    else
    {
        m_velocity.y = -1.f;

        sf::Vector2f tabOffset = { (size.x - tabSize.y) / 2.f,  size.y };
        float tabDent = tabSize.y / 6.f;

        m_vertices.emplace_back(tabOffset, vertColour);
        m_vertices.emplace_back(sf::Vector2f(tabOffset.x + tabSize.y, tabOffset.y), vertColour);
        m_vertices.emplace_back(sf::Vector2f(tabOffset.x + (tabSize.y - tabDent), tabOffset.y + tabSize.x), vertColour);
        m_vertices.emplace_back(sf::Vector2f(tabOffset.x + tabDent, tabOffset.y + tabSize.x), vertColour);

        m_globalBounds.height += tabSize.x;
        m_tabBounds = { tabOffset, {tabSize.y, tabSize.x} };
    }

    //add a message handler to receive mouse clicks
    xy::Component::MessageHandler mh;
    mh.id = Message::Interface;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::InterfaceEvent>();
        if (data.type == Message::InterfaceEvent::MouseClick)
        {
            sf::Vector2f localPoint = m_entity->getWorldTransform().getInverse().transformPoint(data.positionX, data.positionY);
            if(m_tabBounds.contains(localPoint))
            {
                if (!m_moving)
                {
                    m_distanceRemaining = m_travelDistance;
                    m_velocity = -m_velocity;
                    m_moving = true;
                }
            }
        }
    };
    addMessageHandler(mh);
}

//public
void TabComponent::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_moving)
    {
        float distance = moveSpeed * dt;
        m_distanceRemaining -= distance;

        if (m_distanceRemaining < 1)
        {
            distance += m_distanceRemaining;
            m_distanceRemaining = 0.f;
            m_moving = false;
        }

        entity.move(m_velocity * entity.getScale() * distance);
    }
}

//private
void TabComponent::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}