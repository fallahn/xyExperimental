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

#include <MGSelector.hpp>
#include <MessageIDs.hpp>

#include <xygine/MessageBus.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    const std::size_t maxIndex = 2;
}

Selector::Selector(const sf::Texture& t)
    : m_texture     (t),
    m_currentIndex  (0),
    m_target        (0.f)
{
    sf::Vector2f texSize(t.getSize());

    m_topButton.width = texSize.x / 2.f;
    m_topButton.height = texSize.y / 3.f;

    m_bottomButton = m_topButton;
    m_bottomButton.top = m_bottomButton.height * 2.f;

    m_vertices[0].texCoords.x = m_topButton.width;
    m_vertices[0].position.y = m_topButton.height;

    m_vertices[1].texCoords.x = texSize.x;
    m_vertices[1].position = { m_topButton.width, m_topButton.height };

    m_vertices[2].texCoords = { texSize.x, m_topButton.height };
    m_vertices[2].position = { m_topButton.width, m_topButton.height * 2.f };

    m_vertices[3].texCoords = { m_topButton.width, m_topButton.height };
    m_vertices[3].position.y = m_topButton.height * 2.f;

    m_vertices[5].texCoords.x = m_topButton.width;
    m_vertices[5].position.x = m_topButton.width;

    m_vertices[6].texCoords = { m_topButton.width, texSize.y };
    m_vertices[6].position = m_vertices[6].texCoords;

    m_vertices[7].texCoords.y = texSize.y;
    m_vertices[7].position.y = texSize.y;

    m_localBounds.width = m_topButton.width;
    m_localBounds.height = texSize.y;
}

//public
void Selector::update(float dt)
{
    if (m_target != m_vertices[0].texCoords.y)
    {
        float movement = m_target - m_vertices[0].texCoords.y;
        if (std::abs(movement) > 2.f)
        {
            movement *= (dt * 3.f);
        }

        for (auto i = 0u; i < 4u; ++i)
        {
            m_vertices[i].texCoords.y += (movement);
        }
    }
}

void Selector::click(sf::Vector2f clickPos, xy::MessageBus& mb)
{
    if (m_target != m_vertices[0].texCoords.y) return;

    auto point = getInverseTransform().transformPoint(clickPos);
    if (m_bottomButton.contains(point))
    {
        if (m_currentIndex > 0)
        {
            m_currentIndex--;
            m_target = m_vertices[0].texCoords.y - m_bottomButton.height;

            auto msg = mb.post<Message::InterfaceEvent>(Message::Interface);
            msg->type = Message::InterfaceEvent::SelectorClick;
        }
    }
    else if (m_topButton.contains(point))
    {
        if (m_currentIndex < maxIndex)
        {
            m_currentIndex++;
            m_target = m_vertices[0].texCoords.y + m_topButton.height;

            auto msg = mb.post<Message::InterfaceEvent>(Message::Interface);
            msg->type = Message::InterfaceEvent::SelectorClick;
        }
    }
}

//private
void Selector::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &m_texture;

    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}