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

#include <MGWheel.hpp>

#include <xygine/Resource.hpp>
#include <xygine/util/Position.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

Wheel::Wheel(xy::TextureResource& tr)
    : m_wheelBack(tr.get("assets/images/minigames/darts/wheel_back.png")),
    m_wheelFront(tr.get("assets/images/minigames/darts/wheel_front.png")),
    m_currentSpeed(0.f)
{
    xy::Util::Position::centreOrigin(m_wheelBack);
    xy::Util::Position::centreOrigin(m_wheelFront);
    m_wheelFront.setRotation(-90.f);
}

//public
void Wheel::update(float dt)
{
    if (m_currentSpeed != 0)
    {
        m_wheelBack.rotate(m_currentSpeed * dt);
        m_currentSpeed *= 0.99f;

        if (std::abs(m_currentSpeed) < 2.f)
        {
            m_currentSpeed = 0.f;
        }
    }
}

void Wheel::spin(float speed)
{
    m_currentSpeed = speed;
}

std::size_t Wheel::getIndex(std::size_t segCount) const
{
    return std::size_t(std::floor(m_wheelBack.getRotation() / (360.f / static_cast<float>(segCount))));
}

//private
void Wheel::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    rt.draw(m_wheelBack, states);
    rt.draw(m_wheelFront, states);
}