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
#include <ThinkTask.hpp>
#include <MessageIDs.hpp>

#include <xygine/Entity.hpp>


BudController::BudController(xy::MessageBus& mb, const PathFinder& pf, const std::vector<TaskData>& taskData)
    : xy::Component(mb, this),
    m_entity(nullptr),
    m_pathFinder(pf),
    m_taskData(taskData)
{
    //messagehandler takes requests from think task   
    xy::Component::MessageHandler mh;
    mh.id = Message::NewTask;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::TaskEvent>();

        //lookup task data
        auto result = std::find_if(std::begin(m_taskData), std::end(m_taskData), [&data](const TaskData& td)
        {
            return td.id == data.taskName;
        });

        if (result != m_taskData.end())
        {
            //get new destination for requested task and calculate path
            m_destinationPosition = result->position;

            auto points = m_pathFinder.plotPath(m_currentPosition, m_destinationPosition);
            m_tasks.emplace_back(std::make_unique<TravelTask>(*m_entity, getMessageBus(), points));
        }

        //add the requested task
        switch (data.taskName)
        {
        default: break;
        case Message::TaskEvent::Eat:
            LOG("Bud decided to eat!", xy::Logger::Type::Info);
            break;
        case Message::TaskEvent::Drink:
            LOG("Bud decided to drink!", xy::Logger::Type::Info);
            break;
        case Message::TaskEvent::Poop:
            LOG("Bud decided to poop!", xy::Logger::Type::Info);
            break;
        case Message::TaskEvent::Shower:
            LOG("Bud decided to shower!", xy::Logger::Type::Info);
            break;
        case Message::TaskEvent::Sleep:
            LOG("Bud decided to sleep!", xy::Logger::Type::Info);
            break;
        case Message::TaskEvent::WatchTV:
            LOG("Bud decided to watch TV!", xy::Logger::Type::Info);
            break;
        case Message::TaskEvent::PlayPiano:
            LOG("Bud decided to play piano!", xy::Logger::Type::Info);
            break;
        case Message::TaskEvent::PlayMusic:
            LOG("Bud decided to play music!", xy::Logger::Type::Info);
            break;
        case Message::TaskEvent::PlayComputer:
            LOG("Bud decided to play computer!", xy::Logger::Type::Info);
            break;
        }

        //add a new think task to think about what happens when task complete
        m_tasks.emplace_back(std::make_unique<ThinkTask>(*m_entity, getMessageBus()));
    };
    addMessageHandler(mh);
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
}

void BudController::onStart(xy::Entity& entity)
{
    //TODO set current grid position from entity position
    m_currentPosition = { 26u, 29u };
    m_destinationPosition = m_currentPosition;

    //place a ThinkTask on stack first so bud decides what to do
    m_tasks.emplace_back(std::make_unique<ThinkTask>(entity, getMessageBus()));

    m_entity = &entity;
}