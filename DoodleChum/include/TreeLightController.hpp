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

#ifndef DC_TREELIGHT_CONTROLLER_HPP_
#define DC_TREELIGHT_CONTROLLER_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Color.hpp>

#include <array>

namespace xy
{
    class Material;
}

class TreeLightController final : public xy::Component
{
public:
    TreeLightController(xy::MessageBus&, xy::Material&);
    ~TreeLightController() = default;

    xy::Component::Type type() const override { return xy::Component::Type::Script; }
    void entityUpdate(xy::Entity&, float) override;

private:

    xy::Material& m_material;

    std::array<float, 3> m_values;
    std::size_t m_currentIndex;
    bool m_up;
    sf::Color m_colour;
};


#endif //DC_TREELIGHT_CONTROLLER_HPP_