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

#ifndef DC_TASK_HPP_
#define DC_TASK_HPP_

#include <MessageIDs.hpp>

#include <SFML/System/Vector2.hpp>

#include <memory>
#include <string>

namespace xy
{
    class Entity;
    class MessageBus;
}

class Task
{
public:
    using Ptr = std::unique_ptr<Task>;

    explicit Task(xy::Entity& e, xy::MessageBus& mb) : m_entity(e), m_messageBus(mb), m_completed(false) {}
    virtual ~Task() = default;
    Task(const Task&) = delete;
    const Task& operator = (const Task&) = delete;

    virtual void onStart() {}
    virtual void update(float) = 0;
    bool completed() const { return m_completed; }

    virtual Message::TaskEvent::Name getName() const = 0;

protected:
    xy::Entity&  getEntity() { return m_entity; }
    void setCompleted(Message::TaskEvent::Name name)
    {
        m_completed = true; 
        auto msg = m_messageBus.post<Message::TaskEvent>(Message::TaskCompleted);
        msg->taskName = name;
    }
    xy::MessageBus& getMessageBus() { return m_messageBus; }
private:
    xy::Entity& m_entity;
    xy::MessageBus& m_messageBus;
    bool m_completed;
};

struct TaskData final
{
    std::string name;
    std::int32_t id = -1;
    std::int32_t animationID = -1;
    sf::Vector2u position;
    sf::Vector2f worldPosition;
};

#endif //DC_TASK_HPP_