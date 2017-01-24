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

#ifndef DC_MG_SELECTOR_HPP_
#define DC_MG_SELECTOR_HPP_

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <array>

namespace sf
{
    class Texture;
}

class Selector final : public sf::Drawable, public sf::Transformable
{
public:
    explicit Selector(const sf::Texture&);
    ~Selector() = default;

    void update(float);
    void click(sf::Vector2f);

    std::size_t getIndex() const { return m_currentIndex; }

    const sf::FloatRect& getLocalBounds() const { return m_localBounds; }
    sf::FloatRect getGlobalBounds() const { return getTransform().transformRect(m_localBounds); }

private:
    const sf::Texture& m_texture;
    std::size_t m_currentIndex;

    sf::FloatRect m_topButton;
    sf::FloatRect m_bottomButton;

    float m_target;

    sf::FloatRect m_localBounds;

    std::array<sf::Vertex, 8u> m_vertices;
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};


#endif //DC_MG_SELECTOR_HPP_