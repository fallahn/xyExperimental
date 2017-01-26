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
#include <functional>

namespace sf
{
    class Texture;
}

class Dartboard final : public sf::Drawable, public sf::Transformable
{
public:
    explicit Dartboard(const sf::Texture&, const sf::Texture&);
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
    sf::Vector2f m_lastPos;

    std::uint32_t m_score;
    std::uint32_t m_remainingDarts;

    const sf::Texture& m_texture;
    const sf::Texture& m_dartTexture;
    std::array<sf::Vertex, 12u> m_vertices;

    void draw(sf::RenderTarget&, sf::RenderStates) const override;

    class Dart final : public sf::Transformable, public sf::Drawable
    {
    public:
        explicit Dart(const sf::Texture&, sf::Vector2f);
        ~Dart() = default;

        void update(float, sf::Vector2f);
        void fire(sf::Vector2f);
        void addCallback(const std::function<void(const Dart&)>&);

    private:
        const sf::Texture& m_texture;
        float m_rotationSpeed;
        sf::Vector2f m_velocity;
        float m_pauseTime;

        enum class State
        {
            Ready, InFlight, Landed, Spent
        }m_currentState;

        std::array<sf::Vertex, 4u> m_vertices;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;

        std::function<void(const Dart&)> landedCallback;
    };
    std::vector<Dart> m_darts;
    bool m_pendingDart;
    void addDart();
};

#endif //DC_DARTBOARD_HPP_