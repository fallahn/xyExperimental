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

#ifndef ST_CHUNK_HPP_
#define ST_CHUNK_HPP_

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/System/Mutex.hpp>

#include <array>
#include <atomic>

namespace sf
{
    class Shader;
}

using ChunkTexture = std::pair<sf::Texture, bool>;

class Chunk final : public sf::Drawable
{
public:
    Chunk(sf::Vector2f position, sf::Shader&, ChunkTexture&, sf::Shader&, sf::Texture&);
    ~Chunk();

    std::uint64_t getID() const { return m_ID; }

    const sf::FloatRect& getGlobalBounds() const { return m_globalBounds; }
    const sf::Vector2f& getPosition() { return m_position; }

    static const sf::Vector2f& chunkSize();
    static sf::Uint32 chunkTilesSide();

    void update();

    void destroy();
    bool destroyed() const { return m_destroyed; }

    bool isWater(const sf::Vector2f&) const;
    std::uint8_t getBiomeID(const sf::Vector2f&) const;

private:
    
    std::uint64_t m_ID;
    bool m_modified;
    bool m_destroyed;
    sf::Vector2f m_position;

    std::array<sf::Vertex, 4u> m_vertices;
    sf::FloatRect m_globalBounds;


    std::array<std::uint16_t, 4096> m_terrainData;
    std::atomic_bool m_updatePending;
    sf::Thread m_generationThread;
    sf::Mutex m_mutex;

    ChunkTexture& m_chunkTexture;
    sf::Shader& m_terrainShader;

    sf::Texture& m_waterFloorTexture;
    sf::Shader& m_waterShader;
    void draw(sf::RenderTarget&, sf::RenderStates) const override;

    void load();
    void save();

    bool loadFromDisk();
    void generate();
    void updateTexture();
};

#endif //ST_CHUNK_HPP_