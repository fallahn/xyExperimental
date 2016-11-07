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

#ifndef ST_TERRAIN_CONPONENT_HPP_
#define ST_TERRAIN_COMPONENT_HPP_

#include <Chunk.hpp>

#include <xygine/components/Component.hpp>
#include <xygine/detail/ObjectPool.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <vector>
#include <memory>

class TerrainComponent final : public xy::Component, public sf::Drawable 
{
public:
    TerrainComponent(xy::MessageBus&, xy::App&);
    ~TerrainComponent();

    xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
    void entityUpdate(xy::Entity&, float) override;

    sf::FloatRect globalBounds() const override { return (m_currentChunk) ? m_currentChunk->getGlobalBounds() : sf::FloatRect(); }

    static int getSeed();
    static void setSeed(int);

private:
    xy::App& m_appInstance;
    
    std::array<std::pair<sf::Vector2f, bool>, 8> m_radialPoints;

    float m_maxDistance;
    sf::Vector2f m_playerPosition;

    sf::Shader m_waterShader;
    sf::Texture m_waterFloorTexture;
    sf::Texture m_waterReflectionTexture;
    void updateReflectionTexture();

    sf::Texture m_terrainTexture;
    mutable sf::Shader m_terrainShader;
    std::vector<ChunkTexture> m_texturePool;
    ChunkTexture& getChunkTexture();
    void loadTerrainTexture();

    Chunk* m_currentChunk;
    using ChunkPtr = xy::Detail::ObjectPool<Chunk>::Ptr;
    xy::Detail::ObjectPool<Chunk> m_chunkPool; //pool must live longer!
    std::vector<ChunkPtr> m_activeChunks;

    void updateChunks();
    void draw(sf::RenderTarget&, sf::RenderStates) const override;

    void registerWindow();
};

#endif //ST_TERRAIN_COMPONENT_HPP_