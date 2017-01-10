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

#ifndef DC_RAIN_HPP_
#define DC_RAIN_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include<SFML/Graphics/Vertex.hpp>

#include <array>

class RainEffect final : public xy::Component, public sf::Drawable
{
public:
    RainEffect(xy::MessageBus&, sf::Texture&);
    ~RainEffect() = default;

    xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
    void entityUpdate(xy::Entity&, float) override;

private:
    std::array<sf::Vertex, 8u> m_vertices;
    sf::Texture& m_texture;
    float m_transparency;
    bool m_stopped;

    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};

#endif //DC_RAIN_HPP_