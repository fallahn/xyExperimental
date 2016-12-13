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

#include <ValueBar.hpp>

#include <xygine/Assert.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <sstream>

ValueBar::ValueBar(sf::Font& font, const sf::Texture& texture, const sf::Vector2f& size)
    : m_texture (texture),
    m_size      (size),
    m_value     (100.f)
{
    sf::Vector2f texSize = static_cast<sf::Vector2f>(texture.getSize());
    m_vertices =
    {
        sf::Vertex({}, sf::Vector2f()),
        sf::Vertex({size.x, 0.f}, {texSize.x, 0.f}),
        sf::Vertex(size, {texSize.x, texSize.y / 2.f}),
        sf::Vertex({0.f, size.y}, {0.f, texSize.y / 2.f}),
        sf::Vertex({}, {0.f, texSize.y / 2.f}),
        sf::Vertex({size.x, 0.f}, {texSize.x, texSize.y / 2.f}),
        sf::Vertex(size, texSize),
        sf::Vertex({0.f, size.y}, {0.f, texSize.y})
    };

    m_titleText.setFillColor(sf::Color::Black);
    m_titleText.setCharacterSize(32u);
    m_titleText.setPosition(6.f, 6.f);
    m_titleText.setFont(font);

    m_valueText = m_titleText;
    m_valueText.setString(std::to_string(100));
    m_valueText.setPosition(size.x + 12.f, 6.f);
}

//public
void ValueBar::setValue(float value)
{
    value = std::max(0.f, std::min(value, 100.f));
    XY_ASSERT(m_vertices.size() == 8, "vertex data incorrect");

    m_value = value;

    std::stringstream ss;
    ss.precision(4);
    ss /*<< std::stringstream::fixed*/ << value;
    m_valueText.setString(ss.str());
}

void ValueBar::setTitle(const std::string& str)
{
    m_titleText.setString(str);
}

void ValueBar::update(float dt)
{
    const float position = (m_value / 100.f) * m_size.x;

    float diff = position - m_vertices[5].position.x;

    if (std::abs(diff) > 2.f)
    {
        static const float moveSpeed = 400.f;
        m_vertices[5].position.x += diff * dt;
        m_vertices[6].position.x += diff * dt;
    }
    else
    {
        m_vertices[5].position.x = position;
        m_vertices[6].position.x = position;
    }
}

//private
void ValueBar::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);

    states.texture = nullptr;
    rt.draw(m_titleText, states);
    rt.draw(m_valueText, states);
}