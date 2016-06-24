/******************************************************************

Matt Marchant 2016
http://trederia.blogspot.com

xyRacer - Zlib license.

This software is provided 'as-is', without any express or
implied warranty.In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions :

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.

******************************************************************/

#ifndef PS_CAR_B2D_HPP_
#define PS_CAR_B2D_HPP_

#include <xygine/components/Component.hpp>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
    }
}

enum Control
{
    Forward = 0x1,
    Backward = 0x2,
    Left = 0x4,
    Right = 0x8
};

class UserInterface;
class CarControllerB2D final : public xy::Component
{
public:
    CarControllerB2D(xy::MessageBus&, xy::Physics::RigidBody*, UserInterface&);
    ~CarControllerB2D();

    xy::Component::Type type() const override { return xy::Component::Type::Script; }
    void entityUpdate(xy::Entity&, float) override;

    void setInput(sf::Uint8 input) { m_input = input; }

private:

    xy::Physics::RigidBody* m_body;
    UserInterface& m_ui;
    sf::Uint8 m_input;

    sf::Vector2f getDirectionalVelocity(const sf::Vector2f&) const;
};

#endif //PS_CAR_B2D_HPP_