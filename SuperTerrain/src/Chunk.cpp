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
#include <TerrainComponent.hpp>

#include <xygine/util/Vector.hpp>
#include <xygine/util/Math.hpp>
#include <xygine/detail/GLExtensions.hpp>
#include <xygine/detail/GLCheck.hpp>

#include <FastNoiseSIMD.h>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/System/Lock.hpp>

#include <iostream>
#include <algorithm>

using fn = FastNoiseSIMD;

namespace
{
    const sf::Vector2f chunkWorldSize(4096.f, 4096.f);
    const sf::Uint32 chunkTileCount = 64u;

    const std::uint8_t biomeWidth = 40;
    constexpr std::size_t biomeTableSize = biomeWidth * biomeWidth;
    const std::array<std::int8_t, biomeTableSize> biomeTable =
    {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 8,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 8,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 8, 8, 8,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 8, 8, 8,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 8, 8, 8, 8,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 8, 8, 8, 8,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 8, 8, 8, 8,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 8, 8, 8, 8, 8, 8,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 8, 8, 8, 8, 8, 8,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 8, 8, 8, 8, 8, 8,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 8, 8, 8, 8, 4, 4,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 8, 8, 8, 4, 4, 4, 4,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 8, 8, 4, 4, 4, 4, 4,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 8, 8, 4, 4, 4, 4, 4, 4,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 7, 8, 8, 4, 4, 4, 4, 4, 4,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 7, 7, 8, 8, 8, 4, 4, 4, 4, 4, 4,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 7, 7, 7, 8, 8, 4, 4, 4, 4, 4, 4, 4,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 7, 7, 7, 8, 8, 4, 4, 4, 4, 4, 4, 4,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 7, 7, 6, 6, 8, 8, 4, 4, 4, 4, 4, 4, 4,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 7, 6, 6, 6, 6, 8, 3, 4, 4, 4, 4, 4, 4, 4,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 6, 6, 6, 6, 6, 6, 8, 3, 4, 4, 4, 4, 4, 4, 4,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 3, 3, 4, 4, 4, 4, 4, 4, 4,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 5, 7, 7, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 2,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 2, 2,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 2, 2, 2,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 2, 2, 2, 2, 2,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 5, 5, 5, 5, 5, 5, 6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 5, 5, 5, 5, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 5, 5, 5, 5, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        -1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 0, 5, 5, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        -1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 0, 0, 5, 5, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        -1,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        -1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 0, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        -1,-1,-1, 0, 0, 0, 0, 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        -1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
         0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
    };
}

Chunk::Chunk(sf::Vector2f position, sf::Shader& tshader, ChunkTexture& ct, sf::Shader& wshader, sf::Texture& wft)
    : m_ID              (0),
    m_modified          (false),
    m_destroyed         (false),
    m_position          (position),
    m_updatePending     (false),
    m_generationThread  (&Chunk::generate, this),
    m_chunkTexture      (ct),
    m_terrainShader     (tshader),
    m_waterFloorTexture (wft),
    m_waterShader       (wshader)
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

    m_chunkTexture.second = true;

    //generate a UID for chunk based on world position
    std::hash<float> floatHash;
    m_ID = (53 + floatHash(position.x)) * 53 + floatHash(position.y);

    load();
}

Chunk::~Chunk()
{
    m_chunkTexture.second = false;
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
    m_waterShader.setUniform("u_depthTexture", m_chunkTexture.first);
    states.texture = &m_chunkTexture.first;
    states.shader = &m_waterShader;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);

    m_terrainShader.setUniform("u_texture", m_chunkTexture.first);
    //states.texture = &m_chunkTexture.first;
    states.shader = &m_terrainShader;
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
    
    int seed = 0;
    {
        sf::Lock lock(m_mutex);
        seed = TerrainComponent::getSeed();
    }

    auto noise = fn::NewFastNoiseSIMD(seed);
    float* terrainData = noise->GetValueFractalSet(0, int(m_globalBounds.top / chunkWorldSize.y) * chunkTileCount, int(m_globalBounds.left / chunkWorldSize.x) * chunkTileCount,
        chunkTileCount, chunkTileCount, chunkTileCount);
    
    noise->SetFrequency(0.002f);
    float* rainData = noise->GetSimplexSet(0, int(m_globalBounds.top / chunkWorldSize.y) * chunkTileCount, int(m_globalBounds.left / chunkWorldSize.x) * chunkTileCount,
        chunkTileCount, chunkTileCount, chunkTileCount, 0.59f);

    float* tempData = noise->GetGradientSet(0, (int(m_globalBounds.top / chunkWorldSize.y) * chunkTileCount) + chunkTileCount,
        (int(m_globalBounds.left / chunkWorldSize.x) * chunkTileCount) - chunkTileCount,
        chunkTileCount, chunkTileCount, chunkTileCount, 0.26f);

    std::size_t i = 0;
    for (auto y = 0; y < chunkTileCount; ++y)
    {
        for (auto z = 0; z < chunkTileCount; ++z)
        {
            m_terrainData[i] = static_cast<std::uint16_t>(xy::Util::Math::clamp((terrainData[i] * 0.5f + 0.5f), 0.f, 1.f) * 3.f);
            m_terrainData[i] &= 0xFF;

            //std::uint8_t rain = static_cast<std::uint8_t>(xy::Util::Math::clamp((rainData[i] * 0.5 + 0.5f), 0.f, 1.f) * 15.f);
            //m_terrainData[i] |= ((rain & 0x0f) << 8); //for now this is the other noise output - we'll store biome ID here eventually

            //std::uint8_t temp = static_cast<std::uint8_t>(1.f - xy::Util::Math::clamp((tempData[i] * 0.5 + 0.5f), 0.f, 1.f) * 15.f);
            //m_terrainData[i] |= (temp << 12);

            //biome ID is stored in byte 3
            std::uint8_t rain = static_cast<std::uint8_t>((xy::Util::Math::clamp((rainData[i] * 0.5f + 0.5f), 0.f, 1.f) * 399.f) / 10.f);
            std::uint8_t temp = static_cast<std::uint8_t>(xy::Util::Math::clamp((tempData[i] * 0.5f + 0.5f), 0.f, 1.f) * 39.f);
            std::int8_t biome = -1;
            std::size_t index = 0;

            while (biome < 0 && rain < biomeWidth - 2)
            {
                index = rain++ * biomeWidth + temp++;
                biome = biomeTable[index];
            }
            biome = xy::Util::Math::clamp(biome, std::int8_t(0), std::int8_t(8));
            m_terrainData[i] |= ((biome & 0x0f) << 8);

            //depth in byte 4
            float depth = xy::Util::Math::clamp((terrainData[i] * 0.5f + 0.5f) / 0.25f, 0.f, 1.f);
            std::uint8_t d = static_cast<std::uint8_t>(depth * 15.f);
            m_terrainData[i] |= (d << 12);

            i++;
        }
    }

    fn::FreeNoiseSet(tempData);
    fn::FreeNoiseSet(rainData);
    fn::FreeNoiseSet(terrainData);

    save();
    m_updatePending = true;

    //LOG("Thread Quit", xy::Logger::Type::Info);
}

void Chunk::updateTexture()
{
    glCheck(glBindTexture(GL_TEXTURE_2D, m_chunkTexture.first.getNativeHandle()));
    glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, chunkTileCount, chunkTileCount, GL_RED_INTEGER, GL_UNSIGNED_SHORT, (void*)m_terrainData.data()));
    //glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, chunkTileCount, chunkTileCount, GL_RGB_INTEGER, GL_UNSIGNED_SHORT, (void*)m_terrainData.data()));
    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
    m_updatePending = false;
    //m_modified = true;
}