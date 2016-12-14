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

#include <PriceTag.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

PriceTag::PriceTag(sf::Font& font, const sf::Texture& texture)
    : m_texture(texture),
    m_visible(false)
{
    m_text.setFont(font);
    m_text.setPosition(20.f, -5.f);
    m_text.setFillColor(sf::Color::Black);

    sf::Vector2f size = static_cast<sf::Vector2f>(texture.getSize());
    m_vertices[1].position.x = size.x;
    m_vertices[1].texCoords.x = size.x;
    m_vertices[2].position = size;
    m_vertices[2].texCoords = size;
    m_vertices[3].position.y = size.y;
    m_vertices[3].texCoords.y = size.y;
}

//public
void PriceTag::setVisible(bool b)
{
    m_visible = b;
}

void PriceTag::setText(const std::string& str)
{
    m_text.setString(str);
}

//private
void PriceTag::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if (!m_visible) return;

    states.transform *= getTransform();
    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
    rt.draw(m_text, states);
}