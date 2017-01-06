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
#include <SFML/Graphics/Texture.hpp>

namespace
{
    const sf::Vector2f tabSize(40.f, 200.f);
    const float moveSpeed = 2500.f;
    const sf::Color vertColour(255, 255, 255, 250);
    const float easeDistance = 260.f;
}

TabComponent::TabComponent(xy::MessageBus& mb, const sf::Vector2f& size, Direction direction, const sf::Texture& texture)
    : xy::Component     (mb, this),
    m_moving            (false),
    m_travelDistance    (direction == Direction::Horizontal ? size.x : size.y),
    m_distanceRemaining (0.f),
    m_entity            (nullptr),
    m_texture           (texture)
{
    sf::Vector2f textureSize = static_cast<sf::Vector2f>(texture.getSize());
    
    m_vertices.emplace_back(sf::Vector2f(), vertColour);
    m_vertices.emplace_back(sf::Vector2f(size.x, 0.f), vertColour, sf::Vector2f(textureSize.x, 0.f));
    m_vertices.emplace_back(size, vertColour, textureSize);
    m_vertices.emplace_back(sf::Vector2f(0.f, size.y), vertColour, sf::Vector2f(0.f, textureSize.y));

    m_globalBounds.width = size.x;
    m_globalBounds.height = size.y;

    if (direction == Direction::Horizontal)
    {
        m_velocity.x = -1.f; //initial click inverts this
        sf::Vector2f tabOffset = { size.x,  (size.y - tabSize.y) / 2.f };
        float tabDent = tabSize.y / 6.f;

        float texOffset = size.x / (size.x + tabSize.x);
        float texX = textureSize.x * texOffset;
        m_vertices[1].texCoords.x = texX;
        m_vertices[2].texCoords.x = texX;

        texOffset = tabOffset.y / size.y;
        float texY = textureSize.y * texOffset;
        float tabTexHeight = (tabSize.y / size.y) * textureSize.y;
        float tabTexDent = (tabDent / size.y) * textureSize.y;

        m_vertices.emplace_back(tabOffset, vertColour, sf::Vector2f(texX, texY));
        m_vertices.emplace_back(sf::Vector2f(tabOffset.x + tabSize.x, tabOffset.y + tabDent), vertColour, sf::Vector2f(textureSize.x, texY + tabTexDent));
        m_vertices.emplace_back(sf::Vector2f(tabOffset.x + tabSize.x, tabOffset.y + (tabSize.y - tabDent)), vertColour, sf::Vector2f(textureSize.x, texY + (tabTexHeight - tabTexDent)));
        m_vertices.emplace_back(sf::Vector2f(tabOffset.x, tabOffset.y + tabSize.y), vertColour, sf::Vector2f(texX, texY + tabTexHeight));

        m_globalBounds.width += tabSize.x;
        m_tabBounds = { tabOffset, tabSize };
    }
    else
    {
        m_velocity.y = -1.f;

        sf::Vector2f tabOffset = { (size.x - tabSize.y) / 2.f,  size.y };
        float tabDent = tabSize.y / 6.f;

        float texOffset = size.y / (size.y + tabSize.x);
        float texY = texOffset * textureSize.y;
        m_vertices[2].texCoords.y = texY;
        m_vertices[3].texCoords.y = texY;

        texOffset = tabOffset.x / size.x;
        float texX = textureSize.x * texOffset;
        float tabTexWidth = (tabSize.y / size.x) * textureSize.x;
        float tabTexDent = (tabDent / size.x) * textureSize.x;

        m_vertices.emplace_back(tabOffset, vertColour, sf::Vector2f(texX, texY));
        m_vertices.emplace_back(sf::Vector2f(tabOffset.x + tabSize.y, tabOffset.y), vertColour, sf::Vector2f(texX + tabTexWidth, texY));
        m_vertices.emplace_back(sf::Vector2f(tabOffset.x + (tabSize.y - tabDent), tabOffset.y + tabSize.x), vertColour, sf::Vector2f(texX + (tabTexWidth - tabTexDent), textureSize.y));
        m_vertices.emplace_back(sf::Vector2f(tabOffset.x + tabDent, tabOffset.y + tabSize.x), vertColour, sf::Vector2f(texX + tabTexDent, textureSize.y));

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
                toggle();
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

        //easing near end
        if (m_distanceRemaining < easeDistance)
        {
            distance *= (m_distanceRemaining / easeDistance);
        }

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

void TabComponent::toggle()
{
    if (!m_moving)
    {
        m_distanceRemaining = m_travelDistance;
        m_velocity = -m_velocity;
        m_moving = true;

        auto toggleMsg = sendMessage<Message::InterfaceEvent>(Message::Interface);
        toggleMsg->type = Message::InterfaceEvent::TabToggled;
    }
}

//private
void TabComponent::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}