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

#ifndef DC_PATH_FINDER_HPP_
#define DC_PATH_FINDER_HPP_

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>
#include <memory>
#include <set>

class PathFinder final : public sf::Drawable
{
public:
    PathFinder();
    ~PathFinder() = default;

    void setGridSize(const sf::Vector2u& gs) { m_gridSize = static_cast<sf::Vector2i>(gs); }
    void setTileSize(const sf::Vector2f& ts) { m_tileSize = ts; }
    void setGridOffset(const sf::Vector2f& go) { m_gridOffset = go; }
    void addSolidTile(const sf::Vector2u& st);
    bool hasData() const { return !m_soldTiles.empty() && m_gridSize.x != 0 && m_gridSize.y != 0; }

    std::vector<sf::Vector2f> plotPath(const sf::Vector2u&, const sf::Vector2u&) const;

private:
    struct Node final
    {
        using Ptr = std::shared_ptr<Node>;
        Node(const sf::Vector2i& pos, Node::Ptr p = nullptr)
        : position (pos), parent(p), G(0), H(0){}

        sf::Uint32 getScore() const
        {
            return G + H;
        }

        sf::Vector2i position;
        Node::Ptr parent;
        sf::Uint32 G, H;
    };

    sf::Vector2i m_gridSize;
    sf::Vector2f m_tileSize;
    sf::Vector2f m_gridOffset;
    std::vector<sf::Vector2i> m_soldTiles;

    bool collides(const sf::Vector2i&) const;
    Node::Ptr nodeOnList(const std::set<Node::Ptr>&, const sf::Vector2i&) const;

    std::vector<sf::Vertex> m_walls;
    mutable std::vector<sf::Vertex> m_path;
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};

    inline bool operator == (const sf::Vector2i& lh, const sf::Vector2u& rh)
    {
        return (lh.x == static_cast<int>(rh.x) && lh.y == static_cast<int>(rh.y));
    }

    inline bool operator != (const sf::Vector2i& lh, const sf::Vector2u& rh)
    {
        return !(lh == rh);
    }

#endif //DC_PATH_FINDER_HPP_