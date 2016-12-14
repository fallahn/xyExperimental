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

#include <ThinkTask.hpp>
#include <MessageIDs.hpp>
#include <AttributeManager.hpp>

#include <xygine/Log.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/SysTime.hpp>

ThinkTask::ThinkTask(xy::Entity& entity, xy::MessageBus& mb, const AttribManager& am)
    : Task          (entity, mb),
    m_time          (2.f),
    m_attribManager (am)
{
    
}

//public
void ThinkTask::onStart()
{
    auto msg = getMessageBus().post<Message::AnimationEvent>(Message::Animation);
    msg->id = Message::AnimationEvent::Idle;
}

void ThinkTask::update(float dt)
{
    //check for time out, request next task if expired and mark as complete
    m_time -= dt;
    if(m_time <= 0)
    {
        //check our attributes and decide on what to do next   
        auto personalAttribs = m_attribManager.getPersonalAttribs();
        std::sort(std::begin(personalAttribs), std::end(personalAttribs),
            [](const std::pair<std::int32_t, float>& a, const std::pair<std::int32_t, float>& b)
        {
            return a.second > b.second;
        });
        
        //make sure we ignore the health attrib
        std::int32_t attrib = -1;
        std::size_t idx = 0;
        do
        {
            attrib = personalAttribs[idx++].first;
        } while (attrib == AttribManager::Personal::Health || !canDo(attrib));
        

        //perform task based on most dire attrib
        Message::TaskEvent::Name task = Message::TaskEvent::Sleep;
        switch (attrib)
        {
        case AttribManager::Personal::Tiredness:
        default: break;
        case AttribManager::Personal::Boredness:
            //pick whichever is most entertaining
        {
            const auto& hhAtt = m_attribManager.getHouseholdAttribs();
            std::int32_t entertainment = AttribManager::Household::Music;
            for (auto i = 0; i < AttribManager::Household::Count; ++i)
            {
                //a bit kludgy, but fuck it
                if (i == AttribManager::Household::Music ||
                    i == AttribManager::Household::SheetMusic ||
                    i == AttribManager::Household::Games ||
                    i == AttribManager::Household::Films)
                {
                    if (hhAtt[i] > hhAtt[entertainment])
                    {
                        entertainment = i;
                    }
                }
            }

            //raise a message is an entertainment is particularly low
            if (hhAtt[entertainment] < 25)
            {
                auto msg = getMessageBus().post<Message::PlayerEvent>(Message::Player);
                msg->task = entertainment;
                msg->action = Message::PlayerEvent::ResourceLow;
            }

            switch (entertainment)
            {
            default:
            case AttribManager::Household::Music:
                task = Message::TaskEvent::PlayMusic;
                break;
            case AttribManager::Household::SheetMusic:
                task = Message::TaskEvent::PlayPiano;
                break;
            case AttribManager::Household::Games:
                task = Message::TaskEvent::PlayComputer;
                break;
            case AttribManager::Household::Films:
                task = Message::TaskEvent::WatchTV;
                break;
            }
        }
            break;
        case AttribManager::Personal::Hunger:
            task = Message::TaskEvent::Eat;
            break;
        case AttribManager::Personal::Cleanliness:
            task = Message::TaskEvent::Shower;
            break;
        case AttribManager::Personal::Poopiness:
            task = Message::TaskEvent::Poop;
            break;
        case AttribManager::Personal::Thirst:
            task = Message::TaskEvent::Drink;
            break;
        }

        auto msg = getMessageBus().post<Message::TaskEvent>(Message::NewTask);
        msg->taskName = task;
        setCompleted(Message::TaskEvent::Think);
    }
}

//private
bool ThinkTask::canDo(std::int32_t attrib)
{
    const auto& householdAttribs = m_attribManager.getHouseholdAttribs();
    const auto& personalAttribs = m_attribManager.getPersonalAttribs();
    
    switch (attrib)
    {
    default: return false;
    case AttribManager::Personal::Boredness:
        return true; //can always do an activity - it may just not alleviate boredness
    case AttribManager::Personal::Hunger:
    {
        bool possible = (householdAttribs[AttribManager::Household::Food] > 0
            && personalAttribs[AttribManager::Personal::Hunger].second > 50.f);
        if (householdAttribs[AttribManager::Household::Food] == 0)
        {
            auto msg = getMessageBus().post<Message::PlayerEvent>(Message::Player);
            msg->action = Message::PlayerEvent::TaskFailed;
            msg->task = attrib;
        }
        return possible;
    }
    case AttribManager::Personal::Thirst:
        if (personalAttribs[AttribManager::Personal::Thirst].second < 50.f) return false;
    case AttribManager::Personal::Cleanliness:
    case AttribManager::Personal::Poopiness:
    {
        bool possible = householdAttribs[AttribManager::Household::Water] > 0;
        if (!possible)
        {
            auto msg = getMessageBus().post<Message::PlayerEvent>(Message::Player);
            msg->action = Message::PlayerEvent::TaskFailed;
            msg->task = attrib;
        }
        return possible;
    }
    case AttribManager::Personal::Tiredness:
    { //only sleep at night
        const auto& curTime = xy::SysTime::now();
        return (curTime.hours() > 20 || curTime.hours() < 8);
    }
    }
}