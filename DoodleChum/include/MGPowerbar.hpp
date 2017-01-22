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

#ifndef DC_POWERBAR_HPP_
#define DC_POWERBAR_HPP_

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <array>

namespace sf
{
    class Texture;
}

class Powerbar final : public sf::Drawable, public sf::Transformable
{
public:
    explicit Powerbar(const sf::Texture&);
    Powerbar() = default;

    void reset();
    float getValue() const;

    void update(float);

    const sf::FloatRect& getLocalBounds() const { return m_localBounds; }
    sf::FloatRect getGlobalBounds() const { return getTransform().transformRect(m_localBounds); }

private:
    const sf::Texture& m_texture;
    std::array<sf::Vertex, 8u> m_vertices;

    std::size_t m_wavetableIdx;
    sf::FloatRect m_localBounds;

    void draw(sf::RenderTarget&, sf::RenderStates) const override;
    void updateIndicator();
};

#endif //DC_POWERBAR_HPP_