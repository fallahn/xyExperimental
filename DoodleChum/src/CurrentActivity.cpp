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

#include <CurrentActivity.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

CurrentActivity::CurrentActivity(const sf::Texture& texture)
    : m_texture     (texture),
    m_width         (0.f),
    m_travelDest    (0.f)
{
    auto texSize = static_cast<sf::Vector2f>(texture.getSize());
    m_width = texSize.x / 9.f;

    m_vertices[1].position.x = m_width;
    m_vertices[1].texCoords.x = m_width;
    m_vertices[2].position = { m_width, texSize.y };
    m_vertices[2].texCoords = m_vertices[2].position;
    m_vertices[3].position.y = texSize.y;
    m_vertices[3].texCoords.y = texSize.y;
}

//public
void CurrentActivity::update(float dt)
{
    float movement = m_travelDest - m_vertices[0].texCoords.x;
        
    if (std::abs(movement) > 2)
    {
        for (auto& v : m_vertices)
        {
            v.texCoords.x += movement * dt;
        }
    }
    else
    {
        for (auto& v : m_vertices)
        {
            v.texCoords.x += movement;
        }
    }
}

void CurrentActivity::setIndex(std::size_t idx)
{
    m_travelDest = m_width * static_cast<float>(idx);
}

//private
void CurrentActivity::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &m_texture;

    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}