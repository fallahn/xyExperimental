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

#include <TerrainComponent.hpp>

#include <xygine/detail/GLExtensions.hpp>
#include <xygine/detail/GLCheck.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/App.hpp>
#include <xygine/FileSystem.hpp>
#include <xygine/util/Vector.hpp>

#include <xygine/imgui/imgui.h>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    const std::string vertex =
        "#version 120\n"

        "varying vec2 v_texCoord;\n"
        "varying vec4 v_colour;\n"

        "void main()\n"
        "{\n"
        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
        "    v_texCoord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;"
        "    v_colour = gl_Color;\n"
        "}";

    const std::string tileShader =
        "#version 130\n"

        "uniform usampler2D u_texture;\n"

        "in vec2 v_texCoord;\n"
        "in vec4 v_colour;\n"

        "out vec4 colour;\n"

        "vec3[4] colours = vec3[4](vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 0.0), vec3(0.8, 1.0, 0.0), vec3(0.0, 0.9, 0.05));\n"

        "void main()\n"
        "{\n"
        "    uint value = texture(u_texture, v_texCoord).r;\n"
        "    //colour = vec4(vec3(float(value) / 65535.0) * v_colour.rgb, 1.0);\n"
        "    colour = vec4(colours[value], 1.0);\n"
        "}";

    std::size_t maxActiveChunks = 18;
}

TerrainComponent::TerrainComponent(xy::MessageBus& mb)
    :xy::Component  (mb, this),
    m_maxDistance   (xy::Util::Vector::lengthSquared(Chunk::chunkSize() * 1.7f)),
    m_texturePool   (maxActiveChunks),
    m_currentChunk  (nullptr),
    m_chunkPool     (maxActiveChunks)
{
    //radial points used for testing surrounding chunks
    auto length = xy::Util::Vector::length(Chunk::chunkSize());
    m_radialPoints =
    {
        std::make_pair(sf::Vector2f(0.f, 1.f) * length, false),
        std::make_pair(xy::Util::Vector::normalise({-1.f, 1.f}) * length, false),
        std::make_pair(sf::Vector2f(-1.f, 0.f) * length, false),
        std::make_pair(xy::Util::Vector::normalise({ -1.f, -1.f }) * length, false),
        std::make_pair(sf::Vector2f(0.f, -1.f) * length, false),
        std::make_pair(xy::Util::Vector::normalise({ 1.f, -1.f }) * length, false),
        std::make_pair(sf::Vector2f(1.f, 0.f) * length, false),
        std::make_pair(xy::Util::Vector::normalise({ 1.f, 1.f }) * length, false)
    };

    m_shader.loadFromMemory(vertex, tileShader);

    //set up the texture pool
    for (auto& tp : m_texturePool)
    {
        tp.first.create(Chunk::chunkTilesSide(), Chunk::chunkTilesSide());
        glCheck(glBindTexture(GL_TEXTURE_2D, tp.first.getNativeHandle()));
        glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI, tp.first.getSize().x, tp.first.getSize().y, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, 0));
        glCheck(glBindTexture(GL_TEXTURE_2D, 0));
        tp.second = false; //texture not yet used
    }

    //check we have a directory to write chunk data to
    if (!xy::FileSystem::directoryExists("map"))
    {
        xy::FileSystem::createDirectory("map");
    }

#ifdef _DEBUG_
    registerWindow();
#endif //_DEBUG_
}

TerrainComponent::~TerrainComponent()
{
    xy::App::removeUserWindows(this);
}

//public
void TerrainComponent::entityUpdate(xy::Entity& entity, float dt)
{
    m_playerPosition = entity.getScene()->getView().getCenter();

    auto result = std::find_if(std::begin(m_activeChunks), std::end(m_activeChunks),
        [this](const ChunkPtr& chunk)
    {
        return chunk->getGlobalBounds().contains(m_playerPosition);
    });

    bool update = false;
    if (result == m_activeChunks.end() || !m_currentChunk)
    {
        //generate or load new chunk at position
        sf::Vector2f chunkPos = 
        {
            std::floor(m_playerPosition.x / Chunk::chunkSize().x) * Chunk::chunkSize().x,
            std::floor(m_playerPosition.y / Chunk::chunkSize().y) * Chunk::chunkSize().y
        };
        chunkPos += (Chunk::chunkSize() / 2.f);
        m_activeChunks.emplace_back(m_chunkPool.get(chunkPos, m_shader, getTexture()));

        //set current chunk to new chunk
        m_currentChunk = m_activeChunks.back().get();

        update = true;
    }
    else if (result->get() != m_currentChunk)
    {
        //we moved to an existing chunk
        m_currentChunk = result->get();
        update = true;
    }
    if (update) updateChunks();

    //this updates any pending texture changes
    for (auto& c : m_activeChunks) c->update();
}

//private
ChunkTexture& TerrainComponent::getTexture()
{
    return *std::find_if(std::begin(m_texturePool), std::end(m_texturePool), 
        [](const ChunkTexture& ct)
    {
        return !ct.second;
    });
}

void TerrainComponent::updateChunks()
{
    //move around current chunk and delete as necessary
    for (auto& chunk : m_activeChunks)
    {
        if (xy::Util::Vector::lengthSquared(m_currentChunk->getPosition() - chunk->getPosition()) >
            m_maxDistance)
        {
            chunk->destroy();
        }
    }

    //tidy up dead chunks
    m_activeChunks.erase(std::remove_if(std::begin(m_activeChunks), std::end(m_activeChunks), 
        [](const ChunkPtr& chunk)
    {
        return chunk->destroyed();
    }), std::end(m_activeChunks));


    //TODO this could be optimised - so many loops!
    for (auto& point : m_radialPoints) point.second = false;
    auto currentChunkPos = m_currentChunk->getPosition();
    for (const auto& chunk : m_activeChunks)
    {        
        for (auto& point : m_radialPoints)
        {
            if (!point.second && chunk->getGlobalBounds().contains(point.first + currentChunkPos))
            {
                point.second = true;
                break;
            }
        }
    }

    for (const auto& point : m_radialPoints)
    {
        if (!point.second)
        {
            auto currentPos = point.first + currentChunkPos;
            sf::Vector2f chunkPos =
            {
                std::floor(currentPos.x / Chunk::chunkSize().x) * Chunk::chunkSize().x,
                std::floor(currentPos.y / Chunk::chunkSize().y) * Chunk::chunkSize().y
            };
            chunkPos += (Chunk::chunkSize() / 2.f); //account for positioning being in centre
            m_activeChunks.emplace_back(m_chunkPool.get(chunkPos, m_shader, getTexture()));
        }
    }
}

void TerrainComponent::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.shader = &m_shader;
    for (const auto& chunk : m_activeChunks)
    {
        rt.draw(*chunk, states);
    }
}

void TerrainComponent::registerWindow()
{
    xy::App::addUserWindow(
        [this]()
    {
        nim::Begin("Info");
        std::string pp("Player Position: " + std::to_string(m_playerPosition.x) + ", " + std::to_string(m_playerPosition.y));
        nim::Text(pp.c_str());
        if (m_currentChunk)
        {
            pp = "ChunkID: " + std::to_string(m_currentChunk->getID());
            nim::Text(pp.c_str());
            pp = "Chunk Position: " + std::to_string(m_currentChunk->getPosition().x) + ", " + std::to_string(m_currentChunk->getPosition().y);
            nim::Text(pp.c_str());
        }
        pp = "Active chunk count: " + std::to_string(m_activeChunks.size());
        nim::Text(pp.c_str());
        nim::End();
    }, this);
}