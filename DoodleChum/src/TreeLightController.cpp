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

#include <TreeLightController.hpp>

#include <xygine/mesh/Material.hpp>
#include <xygine/mesh/MaterialProperty.hpp>
#include <xygine/Reports.hpp>

namespace
{
    const float minValue = 100.f;
}

TreeLightController::TreeLightController(xy::MessageBus& mb, xy::Material& mat)
    : xy::Component (mb, this),
    m_material      (mat),
    m_values        ({minValue, 255.f, minValue}),
    m_currentIndex  (0),
    m_up            (true)
{

}

//public
void TreeLightController::entityUpdate(xy::Entity&, float dt)
{
    dt *= 10.f;
    
    if (m_up)
    {
        m_values[m_currentIndex] += dt;
        if (m_values[m_currentIndex] > 255)
        {
            m_values[m_currentIndex] = 255.f;
            m_currentIndex = (m_currentIndex + 1) % m_values.size();

            m_up = false;
        }
    }
    else
    {
        m_values[m_currentIndex] -= dt;
        if (m_values[m_currentIndex] < minValue)
        {
            m_values[m_currentIndex] = minValue;
            m_currentIndex = (m_currentIndex + 1) % m_values.size();

            m_up = true;
        }
    }

    m_colour.r = static_cast<sf::Uint8>(m_values[0]);
    m_colour.g = static_cast<sf::Uint8>(m_values[1]);
    m_colour.b = static_cast<sf::Uint8>(m_values[2]);

    m_material.getProperty("u_colour")->setValue(m_colour);
}