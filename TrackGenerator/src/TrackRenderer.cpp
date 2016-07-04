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

#include <TrackRenderer.hpp>
#include <TrackData.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    
}

TrackRenderer::TrackRenderer(xy::MessageBus& mb)
    : xy::Component(mb, this)
{

}

//public
void TrackRenderer::entityUpdate(xy::Entity&, float)
{

}

void TrackRenderer::setData(const TrackData& td)
{
    XY_ASSERT(td.points.size() > 3, "requires at least 4 points");

    //calculates a spline point based on 4 given keys
    std::function<sf::Vector2f(const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f, float)> getPoint = 
        [](const sf::Vector2f& a, const sf::Vector2f& b, const sf::Vector2f& c, const sf::Vector2f d, float t)->sf::Vector2f
    {
        sf::Vector2f retVal;

        const float t2 = t * t;
        const float t3 = t2 * t;

        retVal = 0.5f * ((2.f * b) +
            (-a + c) * t +
            (2.f * a - 5.f * b + 4.f * c - d) * t2 +
            (-a + 3.f * b - 3.f * c + d) * t3);

        return retVal;
    };
    
    //generate vert array as spline
    m_vertices.clear();
    const std::size_t pointsPerCurve = 8;
    for (auto i = 0u; i < td.points.size()/* - 3*/; ++i)
    {
        for (auto j = 0u; j < pointsPerCurve; ++j)
        {
            m_vertices.push_back(getPoint(td.points[i], td.points[(i + 1) % td.points.size()],
                td.points[(i + 2) % td.points.size()], td.points[(i + 3) % td.points.size()], (1.f / pointsPerCurve) * j));
        }
    }
    m_vertices.push_back(m_vertices[0]);


    m_bounds = sf::FloatRect(MAX_AREA, { 0.f, 0.f });
    for (auto v : m_vertices)
    {
        //update bounds
        if (v.position.x < m_bounds.left)
        {
            m_bounds.left = v.position.x;
        }
        else if (v.position.x - m_bounds.left > m_bounds.width)
        {
            m_bounds.width = v.position.x - m_bounds.left;
        }

        if (v.position.y < m_bounds.top)
        {
            m_bounds.top = v.position.y;
        }
        else if (v.position.y - m_bounds.top > m_bounds.height)
        {
            m_bounds.height = v.position.y - m_bounds.top;
        }
    }
}

//private
void TrackRenderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_vertices.data(), m_vertices.size(), sf::PrimitiveType::LinesStrip, states);
}