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

#ifndef DC_WALL_CLOCK_HPP_
#define DC_WALL_CLOCK_HPP_

#include <xygine/components/Component.hpp>
#include <xygine/components/AnimatedDrawable.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Transformable.hpp>

#include <SFML/System/Clock.hpp>

#include <array>

class WallClock final : public xy::Component, public sf::Drawable, public sf::Transformable
{
public:
    WallClock(xy::MessageBus&, const sf::Texture&);
    ~WallClock() = default;

    xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
    void entityUpdate(xy::Entity&, float) override;
    sf::FloatRect globalBounds() const override { return m_globalBounds; }

    const sf::Texture& getTexture() const { return m_texture.getTexture(); }

private:
    sf::FloatRect m_globalBounds;
    sf::Clock m_clock;
    std::array<sf::Vertex, 6u> m_handVertices;
    void updateHands();

    mutable sf::RenderTexture m_texture;
    xy::AnimatedDrawable::Ptr m_sprite;
    void initSprite(const sf::Texture&);

    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};

#endif //DC_WALL_CLOCK_HPP_
