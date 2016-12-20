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

#include <WallClock.hpp>

#include <xygine/util/Vector.hpp>
#include <xygine/SysTime.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    const float degsPerHour = 360.f / 12.f;
    const float degsPerMinute = 360.f / 60.f;
    const sf::Vector2f bigHand(0.f, -40.f);
    const sf::Vector2f smallHand(0.f, -25.f);
}

WallClock::WallClock(xy::MessageBus& mb)
    : xy::Component(mb, this)
{
    for (auto& v : m_vertices)
    {
        v.color = sf::Color::Black;
    }

    updateHands();

    m_globalBounds.left = bigHand.y;
    m_globalBounds.top = bigHand.y;
    m_globalBounds.width = -bigHand.y * 2.f;
    m_globalBounds.height = -bigHand.y * 2.f;
}

//public
void WallClock::entityUpdate(xy::Entity&, float)
{
    if (m_clock.getElapsedTime().asSeconds() > 120.f)
    {
        updateHands();
        m_clock.restart();
    }
}

//private
void WallClock::updateHands()
{
    const auto& time = xy::SysTime::now();
    m_vertices[0].position = xy::Util::Vector::rotate(smallHand, degsPerHour * time.hours());
    m_vertices[2].position = xy::Util::Vector::rotate(bigHand, degsPerMinute * time.minutes());
}

void WallClock::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_vertices.data(), m_vertices.size(), sf::LineStrip, states);
}