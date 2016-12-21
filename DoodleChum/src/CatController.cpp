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

#include <CatController.hpp>
#include <PathFinder.hpp>
#include <CatTravelTask.hpp>
#include <CatAnimationTask.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Random.hpp>

CatController::CatController(xy::MessageBus& mb, const PathFinder& pf, const std::vector<TaskData>& td)
    : xy::Component (mb, this),
    m_entity        (nullptr),
    m_pathFinder    (pf),
    m_taskData      (td)
{

}

//public
void CatController::entityUpdate(xy::Entity&, float dt)
{
    if (!m_tasks.empty())
    {
        m_tasks.front()->update(dt);
        if (m_tasks.front()->completed())
        {
            m_tasks.pop_front();
            if (!m_tasks.empty()) m_tasks.front()->onStart();
        }
    }
    else
    {
        fillTaskStack();
    }

    //TODO update current animation
}

void CatController::onStart(xy::Entity& entity)
{
    m_entity = &entity;    
    
    //pick random start position
    const auto& td = m_taskData[xy::Util::Random::value(0, m_taskData.size() - 1)];
    entity.setWorldPosition(td.worldPosition);
    m_currentPosition = td.position;
    m_destinationPosition = m_currentPosition;
}

//private
void CatController::fillTaskStack()
{     
    //pick a random task for new position and create travel task
    const auto& task = m_taskData[xy::Util::Random::value(0, m_taskData.size() - 1)];
    m_destinationPosition = task.position;

    //if no specific task (such as eat or poop) pick sit or sleep at random
    switch (task.id)
    {
    default:
        m_tasks.emplace_back(std::make_unique<CatAnim>(*m_entity, getMessageBus(),
            (xy::Util::Random::value(0, 1) == 0) ? CatAnim::Action::Sit : CatAnim::Action::Sleep));
        break;
    case 0:
        m_tasks.emplace_back(std::make_unique<CatAnim>(*m_entity, getMessageBus(), CatAnim::Action::Eat));
        break;
    case 1:
        m_tasks.emplace_back(std::make_unique<CatAnim>(*m_entity, getMessageBus(), CatAnim::Action::Poop));
        break;
        //TODO other cases
    }

    auto points = m_pathFinder.plotPath(m_currentPosition, m_destinationPosition);
    m_tasks.emplace_back(std::make_unique<CatTravel>(*m_entity, getMessageBus(), points));


    m_currentPosition = m_destinationPosition;
}