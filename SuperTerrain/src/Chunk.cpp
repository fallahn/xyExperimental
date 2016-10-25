/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

SuperTerrain - Zlib license.

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

#include <Chunk.hpp>

#include <xygine/util/Vector.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    const sf::Vector2f chunkWorldSize(256.f, 256.f);

    const float maxDist = 10000.f;
}

Chunk::Chunk(sf::Vector2f position)
    : m_destroyed   (false),
    m_position      (position)
{
    position -= (chunkWorldSize / 2.f);

    m_globalBounds.left = position.x;
    m_globalBounds.top = position.y;
    m_globalBounds.width = chunkWorldSize.x;
    m_globalBounds.height = chunkWorldSize.y;

    m_vertices[0].position = position;
    m_vertices[1].position = { position.x + chunkWorldSize.x, position.y };
    m_vertices[2].position = position + chunkWorldSize;
    m_vertices[3].position = { position.x , position.y + chunkWorldSize.y };


    float colourRatio = xy::Util::Vector::length(position) / maxDist;
    sf::Uint8 colour = static_cast<sf::Uint8>(colourRatio * 255.f);

    for (auto& v : m_vertices) v.color = sf::Color(colour, colour,colour);
}

//public
const sf::Vector2f& Chunk::chunkSize()
{
    return chunkWorldSize;
}

//private
void Chunk::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}