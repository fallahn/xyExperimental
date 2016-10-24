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

#ifndef ST_TERRAIN_CONPONENT_HPP_
#define ST_TERRAIN_COMPONENT_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>

class TerrainComponent final : public xy::Component, public sf::Drawable 
{
public:
    explicit TerrainComponent(xy::MessageBus&);
    ~TerrainComponent() = default;

    xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
    void entityUpdate(xy::Entity&, float) override;

private:

    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};

#endif //ST_TERRAIN_COMPONENT_HPP_