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
#include <xygine/detail/GLExtensions.hpp>
#include <xygine/detail/GLCheck.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <algorithm>

namespace
{
    const sf::Vector2f chunkWorldSize(256.f, 256.f);
    const sf::Uint32 chunkTileCount = 64u;

    const float maxDist = 10000.f; //just for colouring chunks temporarilililily

    std::array<std::string, 4> quadrants =
    {
        "NorthEast",
        "SouthEast",
        "SouthWest",
        "NorthWest"
    };
}

Chunk::Chunk(sf::Vector2f position, sf::Shader& shader)
    : m_ID          (0),
    m_modified      (false),
    m_destroyed     (false),
    m_position      (position),
    m_shader        (shader)
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

    //create a texture up front - we'll update this later as necessary
    //TODO probably needs to be an int-texture
    if (!m_texture.create(chunkTileCount, chunkTileCount))
    {
        xy::Logger::log("Failed creating empty texture for chunk", xy::Logger::Type::Error);
    }
    std::memset(m_terrainData.data(), 65535, m_terrainData.size());

    //generate a UID for chunk based on world position
    std::hash<std::string> hasher;
    std::size_t quadID = (position.x > 0) ?
        (position.y > 0) ? 1 : 0 :
        (position.y > 0) ? 2 : 3;

    m_ID = std::uint64_t(position.x * position.y) + hasher(quadrants[quadID]);

    load();
}

//public
const sf::Vector2f& Chunk::chunkSize()
{
    return chunkWorldSize;
}

sf::Uint32 Chunk::chunkTilesSide()
{
    return chunkTileCount;
}

void Chunk::destroy()
{
    m_destroyed = true;
    if (m_modified)
    {
        save();
    }
}

//private
void Chunk::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    m_shader.setUniform("u_texture", m_texture);
    states.texture = &m_texture;
    states.shader = &m_shader;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}

void Chunk::load()
{
    //try loading from disk
    if (!loadFromDisk())
    {
        //if failed, generate and mark as modified
        generate();
    }
    updateTexture();
}

void Chunk::save()
{
    //write chunk to disk
}

bool Chunk::loadFromDisk()
{
    return false;
}

void Chunk::generate()
{

    
}

void Chunk::updateTexture()
{
    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture.getNativeHandle()));
    //TODO we only need to call this next line once when converting type
    glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI, m_texture.getSize().x, m_texture.getSize().y, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, 0));
    glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, chunkTileCount, chunkTileCount, GL_RED_INTEGER, GL_UNSIGNED_SHORT, (void*)m_terrainData.data()));
    glCheck(glBindTexture(GL_TEXTURE_2D, 0));

    m_modified = true;
}