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
#include <EatTask.hpp>
#include <DrinkTask.hpp>
#include <PoopTask.hpp>
#include <ShowerTask.hpp>
#include <SleepTask.hpp>
#include <TVTask.hpp>
#include <PianoTask.hpp>
#include <MusicTask.hpp>
#include <ComputerTask.hpp>
#include <MessageIDs.hpp>

#include <xygine/Entity.hpp>


BudController::BudController(xy::MessageBus& mb, const PathFinder& pf, const std::vector<TaskData>& taskData, const sf::Texture& spriteSheet)
    : xy::Component (mb, this),
    m_entity        (nullptr),
    m_pathFinder    (pf),
    m_taskData      (taskData),
    m_spriteSheet   (spriteSheet)
{
    //set up render texture ready for animations
    initSprite();

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
            m_tasks.emplace_back(std::make_unique<EatTask>(*m_entity, getMessageBus()));
            break;
        case Message::TaskEvent::Drink:
            LOG("Bud decided to drink!", xy::Logger::Type::Info);
            //two frame - repeat 4/4 raised/lowered
            m_tasks.emplace_back(std::make_unique<DrinkTask>(*m_entity, getMessageBus()));
            break;
        case Message::TaskEvent::Poop:
            LOG("Bud decided to poop!", xy::Logger::Type::Info);
            //one frame?
            m_tasks.emplace_back(std::make_unique<PoopTask>(*m_entity, getMessageBus()));
            break;
        case Message::TaskEvent::Shower:
            LOG("Bud decided to shower!", xy::Logger::Type::Info);
            //scale 0, steam particle effect
            m_tasks.emplace_back(std::make_unique<ShowerTask>(*m_entity, getMessageBus()));
            break;
        case Message::TaskEvent::Sleep:
            LOG("Bud decided to sleep!", xy::Logger::Type::Info);
            //scale 0, ZZzz particle effect
            m_tasks.emplace_back(std::make_unique<SleepTask>(*m_entity, getMessageBus()));
            break;
        case Message::TaskEvent::WatchTV:
            LOG("Bud decided to watch TV!", xy::Logger::Type::Info);
            //sitting with remote animation?
            m_tasks.emplace_back(std::make_unique<TVTask>(*m_entity, getMessageBus()));
            break;
        case Message::TaskEvent::PlayPiano:
            LOG("Bud decided to play piano!", xy::Logger::Type::Info);
            //animation
            m_tasks.emplace_back(std::make_unique<PianoTask>(*m_entity, getMessageBus()));
            break;
        case Message::TaskEvent::PlayMusic:
            LOG("Bud decided to play music!", xy::Logger::Type::Info);
            //dancing animation with note particle effect
            m_tasks.emplace_back(std::make_unique<MusicTask>(*m_entity, getMessageBus()));
            break;
        case Message::TaskEvent::PlayComputer:
            LOG("Bud decided to play computer!", xy::Logger::Type::Info);
            //probably recycle piano animation
            m_tasks.emplace_back(std::make_unique<ComputerTask>(*m_entity, getMessageBus()));
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
            if (!m_tasks.empty()) m_tasks.front()->onStart();
            m_currentPosition = m_destinationPosition;
        }
    }
    m_sprite->entityUpdate(entity, dt);
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

//private
void BudController::initSprite()
{   
    m_sprite = xy::Component::create<xy::AnimatedDrawable>(getMessageBus(), m_spriteSheet);
    m_sprite->loadAnimationData("assets/images/sprites/bud.xya");
    m_sprite->playAnimation(2);
    auto frameSize = m_sprite->getFrameSize();
    m_sprite->setScale(1.f, -1.f);
    m_sprite->setOrigin(0.f, static_cast<float>(frameSize.y));

    m_texture.create(frameSize.x, frameSize.y);

    //add a message handler to respond to animation changes
    xy::Component::MessageHandler mh;
    mh.id = Message::Animation;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::AnimationEvent>();
        m_sprite->playAnimation(data.id);
    };
    addMessageHandler(mh);
}

void BudController::draw(sf::RenderTarget&, sf::RenderStates) const
{
    m_texture.clear(sf::Color::Transparent);
    m_texture.draw(*m_sprite);
    m_texture.display();
}