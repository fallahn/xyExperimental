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

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/util/Vector.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

TerrainComponent::TerrainComponent(xy::MessageBus& mb)
    :xy::Component  (mb, this),
    m_maxDistance   (xy::Util::Vector::lengthSquared(Chunk::chunkSize()) * 1.1f)
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
}

//public
void TerrainComponent::entityUpdate(xy::Entity& entity, float dt)
{
    auto playerPosition = entity.getScene()->getView().getCenter();

    auto result = std::find_if(std::begin(m_activeChunks), std::end(m_activeChunks),
        [playerPosition](const std::unique_ptr<Chunk>& chunk)
    {
        return chunk->getGlobalBounds().contains(playerPosition);
    });

    bool update = false;
    if (result == m_activeChunks.end() || !m_currentChunk)
    {
        //generate or load new chunk at position
        sf::Vector2f chunkPos = 
        {
            std::floor(playerPosition.x / Chunk::chunkSize().x) * Chunk::chunkSize().x,
            std::floor(playerPosition.y / Chunk::chunkSize().y) * Chunk::chunkSize().y
        };
        m_activeChunks.emplace_back(std::make_unique<Chunk>(chunkPos));

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
}

//private
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
    m_activeChunks.erase( std::remove_if(std::begin(m_activeChunks), std::end(m_activeChunks), 
        [](const std::unique_ptr<Chunk>& chunk)
    {
        return chunk->destroyed();
    }), std::end(m_activeChunks));


    //TODO this could be optimised - so many loops!
    for (auto& point : m_radialPoints) point.second = false;

    for (const auto& chunk : m_activeChunks)
    {
        auto chunkPos = m_currentChunk->getPosition();
        for (auto& point : m_radialPoints)
        {
            if (!point.second && chunk->getGlobalBounds().contains(point.first + chunkPos))
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
            auto currentPos = point.first + m_currentChunk->getPosition();
            sf::Vector2f chunkPos =
            {
                std::floor(currentPos.x / Chunk::chunkSize().x) * Chunk::chunkSize().x,
                std::floor(currentPos.y / Chunk::chunkSize().y) * Chunk::chunkSize().y
            };
            m_activeChunks.emplace_back(std::make_unique<Chunk>(chunkPos));
        }
    }
}

void TerrainComponent::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    for (const auto& chunk : m_activeChunks)
    {
        rt.draw(*chunk, states);
    }
}