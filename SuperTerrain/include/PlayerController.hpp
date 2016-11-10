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

#ifndef  ST_PLAYER_CONTROLLER_HPP_

#include <xygine/components/Component.hpp>

namespace st
{
    class PlayerController final : public xy::Component
    {
    public:
        enum
        {
            Left = 0x1,
            Right = 0x2,
            Forward = 0x4,
            Back = 0x8
        };

        explicit PlayerController(xy::MessageBus&);
        PlayerController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;

        void onStart(xy::Entity&) override;

        void setInput(sf::Uint32 input) { m_lastInput = m_input; m_input = input; }

    private:
        xy::Entity* m_entity;
        sf::Uint32 m_input, m_lastInput;
    };
}

#endif //ST_PLAYER_CONTROLLER_HPP_
