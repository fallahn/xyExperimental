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

#include <PathFinder.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <array>

namespace
{
    const std::array<sf::Vector2i, 8> directions = 
    {
        sf::Vector2i(0, 1),
        sf::Vector2i(1, 0),
        sf::Vector2i(0, -1),
        sf::Vector2i(-1, 0),
        sf::Vector2i(-1, -1),
        sf::Vector2i(1, 1),
        sf::Vector2i(-1, 1),
        sf::Vector2i(1, -1),
    };

    sf::Uint8 heuristic(const sf::Vector2i& start, const sf::Vector2i& end)
    {
        sf::Vector2u delta(std::abs(start.x - end.x), std::abs(start.y - end.y));
        return 10 * delta.x + delta.y;
    }
}

PathFinder::PathFinder()
{

}

//public
void PathFinder::addSolidTile(const sf::Vector2u& tileCoord)
{
    m_soldTiles.push_back(static_cast<sf::Vector2i>(tileCoord));

    m_walls.emplace_back(m_gridOffset + sf::Vector2f(m_tileSize.x * tileCoord.x, m_tileSize.y * tileCoord.y), sf::Color::Black);
    m_walls.emplace_back(m_gridOffset + sf::Vector2f(m_tileSize.x * tileCoord.x, m_tileSize.y * tileCoord.y), sf::Color::Black);
    m_walls.back().position.x += m_tileSize.x;
    m_walls.emplace_back(m_gridOffset + sf::Vector2f(m_tileSize.x * tileCoord.x, m_tileSize.y * tileCoord.y), sf::Color::Black);
    m_walls.back().position += m_tileSize;
    m_walls.emplace_back(m_gridOffset + sf::Vector2f(m_tileSize.x * tileCoord.x, m_tileSize.y * tileCoord.y), sf::Color::Black);
    m_walls.back().position.y += m_tileSize.y;
}

std::vector<sf::Vector2f> PathFinder::plotPath(const sf::Vector2u& start, const sf::Vector2u& end) const
{
    Node::Ptr currentNode;
    std::set<Node::Ptr> openSet;
    std::set<Node::Ptr> closedSet;
    openSet.insert(std::make_shared<Node>(static_cast<sf::Vector2i>(start)));

    while (!openSet.empty())
    {
        currentNode = *openSet.begin();

        for (const auto& node : openSet)
        {
            if (node->getScore() <= currentNode->getScore())
            {
                currentNode = node;
            }
        }

        if (currentNode->position == end)
        {
            break;//we're at the end!
        }

        closedSet.insert(currentNode);
        openSet.erase(std::find(std::begin(openSet), std::end(openSet), currentNode));

        for (auto i = 0u; i < directions.size(); ++i)
        {
            auto coords = currentNode->position + directions[i];

            if (collides(coords) || nodeOnList(closedSet, coords))
            {
                continue;
            }

            auto cost = currentNode->G + (i < 4u) ? 10u : 14u;
            auto nextNode = nodeOnList(openSet, coords);
            if (!nextNode)
            {
                nextNode = std::make_shared<Node>(coords, currentNode);
                nextNode->G = cost;
                nextNode->H = heuristic(nextNode->position, static_cast<sf::Vector2i>(end));
                openSet.insert(nextNode);
            }
            else if (cost < nextNode->G)
            {
                nextNode->parent = currentNode;
                nextNode->G = cost;
            }
        }
    }

    //climb tree to get our path
    std::vector<sf::Vector2f> points;
    while (currentNode)
    {
        points.emplace_back(currentNode->position.x * m_tileSize.x, (currentNode->position.y * m_tileSize.y) + m_tileSize.y);
        points.back() += m_gridOffset;
        currentNode = currentNode->parent;
    }

    //and update vis for debug
    m_path.clear();
    for (const auto& p : points)
    {
        m_path.emplace_back(p, sf::Color::Blue);
    }

    return std::move(points);
}

//private
bool PathFinder::collides(const sf::Vector2i& position) const
{
    return (position.x < 0 || position.x >= m_gridSize.x
        || position.y < 0 || position.y >= m_gridSize.y
        || (std::find(std::begin(m_soldTiles), std::end(m_soldTiles), position) != m_soldTiles.end()));
}

PathFinder::Node::Ptr PathFinder::nodeOnList(const std::set<Node::Ptr>& list, const sf::Vector2i& position) const
{
    auto result = std::find_if(std::begin(list), std::end(list), [&position](const Node::Ptr& node) {return node->position == position; });
    return (result == list.end()) ? nullptr : *result;
}

void PathFinder::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    //rt.draw(m_walls.data(), m_walls.size(), sf::Quads, states);
    rt.draw(m_path.data(), m_path.size(), sf::LineStrip, states);
}