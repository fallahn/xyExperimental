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

#include <xygine/util/Random.hpp>
#include <xygine/util/Const.hpp>

#include <TrackGenerator.hpp>

TrackGenerator::TrackGenerator()
{

}

//public
void TrackGenerator::generate(const Parameters& params)
{
    auto pointCount = xy::Util::Random::value(params.minPoints, params.maxPoints);   
    m_trackData.points.resize(pointCount);

    //TODO set some limit on world/track size and make this centre
    m_trackData.points[0] = { 500.f, 500.f };

    //create the random points using the determent of the
    //last angle to steer the next section
    float direction = 0.f;
    const float maxAngle = params.maxAngle * xy::Util::Const::degToRad;
    for (auto i = 1; i < pointCount; ++i)
    {
        float length = xy::Util::Random::value(params.minSegmentLength, params.maxSegmentLength);
        //determents
        float dx = std::sin(direction) * length;
        float dy = std::cos(direction) * length;

        float x = m_trackData.points[i - 1].x + dx;
        float y = m_trackData.points[i - 1].y + dy;
        m_trackData.points[i] = { x, y };

        float turn = std::pow(xy::Util::Random::value(0.f, 1.f), 1.f / params.curviness);
        if (xy::Util::Random::value(1, 2) % 2 == 0)
        {
            turn = -turn;
        }
        direction += turn * maxAngle;
    }

    //update the last quarter to force points toward joining the start point
    auto lastQ = pointCount / 4 * 3;
    auto currentPoint = pointCount - lastQ;
    float destX = m_trackData.points[0].x;
    float destY = m_trackData.points[0].y;

    for (auto i = lastQ; i < pointCount; ++i)
    {
        float x = m_trackData.points[i].x;
        float y = m_trackData.points[i].y;
        auto a = i - lastQ;
        float aOverCurrent = static_cast<float>(a) / currentPoint;
        m_trackData.points[i].x = destX * aOverCurrent + x * (1.f - aOverCurrent);
        m_trackData.points[i].y = destY * aOverCurrent + y * (1.f - aOverCurrent);
    }
    m_trackData.points.push_back(m_trackData.points.front());

    //calc the min/max points of the bounds
}
