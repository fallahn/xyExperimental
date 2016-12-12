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

#ifndef DC_TAB_COMPONENT_HPP_
#define DC_TAB_COMPONENT_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>

class TabComponent final : public xy::Component, public sf::Drawable
{
public:
    enum class Direction
    {
        Horizontal, Vertical
    };

    TabComponent(xy::MessageBus&, const sf::Vector2f&, Direction, const sf::Texture&);
    ~TabComponent() = default;

    xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
    void entityUpdate(xy::Entity&, float) override;
    void onStart(xy::Entity& e) { m_entity = &e; }

    sf::FloatRect globalBounds() const override { return m_globalBounds; }

private:

    bool m_moving;

    sf::Vector2f m_velocity;
    float m_travelDistance;
    float m_distanceRemaining;
    xy::Entity* m_entity;

    sf::FloatRect m_globalBounds;
    sf::FloatRect m_tabBounds;

    const sf::Texture& m_texture;
    std::vector<sf::Vertex> m_vertices;
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};

    template <typename T>
    sf::Vector2<T> operator * (const sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs)
    {
        return{ lhs.x * rhs.x , lhs.y * rhs.y };
    }
#endif //DC_TAB_COMPONENT_HPP_