/*********************************************************************
Matt Marchant 2017
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

#ifndef DC_DARTBOARD_HPP_
#define DC_DARTBOARD_HPP_

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <array>
#include <vector>

namespace sf
{
    class Texture;
}

class Dartboard final : public sf::Drawable, public sf::Transformable
{
public:
    explicit Dartboard(const sf::Texture&);
    ~Dartboard() = default;

    void update(float, sf::Vector2f);
    std::uint32_t getScore() const { return m_score; }
    std::uint32_t getDartsRemaining() const { return m_remainingDarts; }

    void fire();
    void showCrosshair(bool);


private:

    std::vector<float> m_xOffsets;
    std::vector<float> m_yOffsets;
    std::vector<float> m_amplitudeModifier;
    std::size_t m_xIdx;
    std::size_t m_yIdx;
    std::size_t m_ampIdx;

    sf::Vector2f m_boardSize;
    sf::Vector2f m_mousePos;

    std::uint32_t m_score;
    std::uint32_t m_remainingDarts;

    const sf::Texture& m_texture;
    std::array<sf::Vertex, 12u> m_vertices;

    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};

#endif //DC_DARTBOARD_HPP_