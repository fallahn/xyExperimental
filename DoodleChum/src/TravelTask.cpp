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

#include <TravelTask.hpp>

#include <xygine/util/Vector.hpp>
#include <xygine/Entity.hpp>

namespace
{
    const float minDistance = 100.f;
    const float moveSpeed = 220.f;
}

TravelTask::TravelTask(xy::Entity& entity, std::vector<sf::Vector2f>& points)
    : Task(entity),
    m_points(std::move(points))
{

}

//public
void TravelTask::update(float dt)
{
    auto direction = m_points.back() - getEntity().getWorldPosition();
    auto distance = xy::Util::Vector::lengthSquared(direction);

    if (distance < minDistance)
    {
        m_points.pop_back();
        if (m_points.empty())
        {
            setCompleted();
        }        
    }
    else //if(distance != 0)
    {
        getEntity().move(xy::Util::Vector::normalise(direction) * moveSpeed * dt);
    }
}