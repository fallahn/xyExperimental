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

#include <BudController.hpp>
#include <PathFinder.hpp>
#include <TravelTask.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Random.hpp>


BudController::BudController(xy::MessageBus& mb, const PathFinder& pf, const std::vector<sf::Vector2u>& waypoints)
    : xy::Component(mb, this),
    m_pathFinder(pf),
    m_wayPoints(waypoints)
{
    m_currentPosition = { 26u, 29u };
}

//public
void BudController::entityUpdate(xy::Entity& entity, float dt)
{
    if (!m_tasks.empty())
    {
        m_tasks.front()->update(dt);
        if (m_tasks.front()->completed())
        {
            m_tasks.pop_front();
            m_currentPosition = m_destinationPosition;
        }
    }
    else
    {
        //just for testing create a task to travel somewhere at random
        m_destinationPosition = m_wayPoints[xy::Util::Random::value(0, m_wayPoints.size() - 1)];

        auto points = m_pathFinder.plotPath(m_currentPosition, m_destinationPosition);
        m_tasks.emplace_back(std::make_unique<TravelTask>(entity, points));
    }
}