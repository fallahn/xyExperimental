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

#include <FastNoiseSIMD.h>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <iostream>
#include <algorithm>

using fn = FastNoiseSIMD;

namespace
{
    const sf::Vector2f chunkWorldSize(512.f, 512.f);
    const sf::Uint32 chunkTileCount = 64u;
}

Chunk::Chunk(sf::Vector2f position, sf::Shader& shader, ChunkTexture& ct)
    : m_ID              (0),
    m_modified          (false),
    m_destroyed         (false),
    m_position          (position),
    m_texture           (ct),
    m_updatePending     (false),
    m_generationThread  (&Chunk::generate, this),
    m_shader            (shader)
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

    m_vertices[1].texCoords = { float(chunkTileCount), 0.f };
    m_vertices[2].texCoords = { float(chunkTileCount), float(chunkTileCount) };
    m_vertices[3].texCoords = { 0.f, float(chunkTileCount) };

    m_texture.second = true;

    //generate a UID for chunk based on world position
    std::hash<float> floatHash;
    m_ID = (53 + floatHash(position.x)) * 53 + floatHash(position.y);

    load();
}

Chunk::~Chunk()
{
    m_texture.second = false;
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

void Chunk::update()
{
    if (m_updatePending)
    {
        updateTexture();
    }
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
    m_shader.setUniform("u_texture", m_texture.first);
    states.texture = &m_texture.first;
    states.shader = &m_shader;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}

void Chunk::load()
{
    //try loading from disk
    if (!loadFromDisk())
    {
        //if failed, generate and mark as modified
        m_generationThread.launch();
        for (auto& v : m_vertices) v.color = sf::Color::Green;
    }
}

void Chunk::save()
{    
    //write chunk to disk
    std::ofstream file("map/" + std::to_string(m_ID), std::ios::binary | std::ios::out);
    if (/*file.is_open() && */file.good())
    {
        file.write(reinterpret_cast<const char*>(m_terrainData.data()), std::streamsize(m_terrainData.size() * sizeof(std::uint16_t)));
        //LOG(std::to_string(m_ID), xy::Logger::Type::Info);
    }
    file.close();
}

bool Chunk::loadFromDisk()
{
    std::ifstream file("map/" + std::to_string(m_ID), std::ios::binary | std::ios::in);
    
    if (file.is_open() && file.good())
    {
        file.read(reinterpret_cast<char*>(m_terrainData.data()), std::streamsize(m_terrainData.size() * sizeof(std::uint16_t)));
        file.close();
        updateTexture();
        //LOG(std::to_string(m_ID), xy::Logger::Type::Info);
        for (auto& v : m_vertices) v.color = sf::Color::Blue;
        return true;
    }
    
    return false;
}

void Chunk::generate()
{
    //wait for any previous updates to be completed
    while (m_updatePending) {}
    
    auto noise = fn::NewFastNoiseSIMD();
    float* noiseData = noise->GetValueFractalSet(0, int(m_globalBounds.top / chunkWorldSize.y) * chunkTileCount, int(m_globalBounds.left / chunkWorldSize.x) * chunkTileCount,
        chunkTileCount, chunkTileCount, chunkTileCount);
    
    int i = 0;
    for (auto y = 0; y < chunkTileCount; ++y)
    {
        for (auto z = 0; z < chunkTileCount; ++z)
        {
            m_terrainData[i] = static_cast<std::uint16_t>((noiseData[i++] + 1.f * 0.5f) * 65535.f);
        }
    }

    fn::FreeNoiseSet(noiseData);

    save();
    m_updatePending = true;

    //LOG("Thread Quit", xy::Logger::Type::Info);
}

void Chunk::updateTexture()
{
    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture.first.getNativeHandle()));
    glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, chunkTileCount, chunkTileCount, GL_RED_INTEGER, GL_UNSIGNED_SHORT, (void*)m_terrainData.data()));
    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
    m_updatePending = false;
    //m_modified = true;
}