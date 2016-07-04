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
#include <xygine/util/Math.hpp>
#include <xygine/util/Vector.hpp>

#include <TrackGenerator.hpp>

#include <SFML/Graphics/Rect.hpp>

TrackGenerator::TrackGenerator()
{

}

//public
void TrackGenerator::generate(const Parameters& params)
{
    auto pointCount = xy::Util::Random::value(params.minPoints, params.maxPoints);   
    m_trackData.points.resize(pointCount);

    //centre of area
    m_trackData.points[0] = MAX_AREA / 2.f;

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
    
    if (params.noCrossing)
    {
        //convert the path to a hull
        createHull();
    }
    m_trackData.points.push_back(m_trackData.points.front());

    //calc the bounds
    auto bounds = sf::FloatRect(MAX_AREA, { 0.f, 0.f });
    for (const auto& point : m_trackData.points)
    {
        if (point.x < bounds.left)
        {
            bounds.left = point.x;
        }
        else if (point.x - bounds.left > bounds.width)
        {
            bounds.width = point.x - bounds.left;
        }

        if (point.y < bounds.top)
        {
            bounds.top = point.y;
        }
        else if (point.y - bounds.top > bounds.height)
        {
            bounds.height = point.y - bounds.top;
        }
    }

    //shift the track to best fit max area
    sf::Vector2f shift(bounds.left, bounds.top);
    shift.x -= ((MAX_AREA.x - bounds.width) / 2.f);
    shift.y -= ((MAX_AREA.y - bounds.height) / 2.f);
    for (auto& p : m_trackData.points)
    {
        p -= shift;
        p.x = xy::Util::Math::clamp(p.x, 0.f, MAX_AREA.x);
        p.y = xy::Util::Math::clamp(p.y, 0.f, MAX_AREA.y);
    }
}

//private
void TrackGenerator::createHull()
{
    //sort by x distance, sub sort by y
    std::sort(m_trackData.points.begin(), m_trackData.points.end(), []
    (const sf::Vector2f& a, const sf::Vector2f& b)
    {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });

    //calcs cross product of OA OB. returns negative value for clockwise turn
    //positive for CCW and 0 if vectors are parallel
    std::function<float(const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&)> cross = 
        [](const sf::Vector2f& o, const sf::Vector2f& a, const sf::Vector2f& b)
    {
        return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
    };

    std::size_t size = m_trackData.points.size();
    std::size_t j = 0;

    std::vector<sf::Vector2f> results(size * 2);

    //build lower hull
    for (auto i = 0u; i < size; ++i)
    {
        while (j >= 2 && cross(results[j - 2], results[j - 1], m_trackData.points[i]) <= 0)
        {
            j--;
        }
        results[j++] = m_trackData.points[i];
    }

    //build upper hull
    for (int i = size - 2, t = j + 1; i >= 0; --i)
    {
        while (j >= t && cross(results[j - 2], results[j - 1], m_trackData.points[i]) <= 0)
        {
            j--;
        }
        results[j++] = m_trackData.points[i];
    }

    results.resize(j - 1);


    //find long straights and split them
    const float minLength = 5000.f;
    std::vector<sf::Vector2f> resizedPoints(results.size() * 2);
    for (auto i = 0u, j = 0u; i < results.size() - 1; ++i)
    {
        resizedPoints[j++] = results[i];
        
        auto segment = results[i + 1] - results[i];
        if (xy::Util::Vector::length(segment) > minLength)
        {
            LOG("Split track segment", xy::Logger::Type::Info);
            //split and offset          
            auto offset = xy::Util::Vector::normalise(segment) * 1000.f; //TODO hook this up as a var somewhere
            offset = xy::Util::Vector::rotate(offset, xy::Util::Random::value(0.f, 180.f) - 90.f);
            auto newPoint = (segment / 2.f) + offset;
            resizedPoints[j++] = results[i] + newPoint;
        }
    }

    resizedPoints.resize(j - 1);
    m_trackData.points.swap(resizedPoints);
}