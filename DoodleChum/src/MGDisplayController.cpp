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

#include <MGDisplayController.hpp>
#include <MessageIDs.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Console.hpp>

namespace
{
    const sf::Vector2f inTarget(xy::DefaultSceneSize / 2.f);
    const sf::Vector2f outTarget = inTarget + sf::Vector2f(0.f, xy::DefaultSceneSize.y);
}

DisplayController::DisplayController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_moving    (false),
    m_target    (outTarget)
{
#ifdef _DEBUG_

    xy::Console::addCommand("show_minigame", [this](const std::string& param)
    {
        if (param == "0")
        {
            show(false);
        }
        else
        {
            show(true);
        }
    }, this);

#endif //_DEBUG_

    //TODO message handler for close button
}

DisplayController::~DisplayController()
{
    xy::Console::unregisterCommands(this);
}

//public
void DisplayController::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_moving)
    {
        auto pos = entity.getPosition();
        auto distance = m_target - pos;
        if (xy::Util::Vector::lengthSquared(distance) < 2.f)
        {
            entity.setPosition(m_target);
            m_moving = false;

            if (m_target == inTarget)
            {
                auto msg = sendMessage<Message::InterfaceEvent>(Message::Interface);
                msg->type = Message::InterfaceEvent::MiniGameOpen;
            }
        }
        else
        {
            entity.move(distance * (dt * 4.f));
        }
    }
}

void DisplayController::show(bool show)
{
    if (!m_moving)
    {
        m_target = (show) ? inTarget : outTarget;
        m_moving = true;

        if (!show)
        {
            auto msg = sendMessage<Message::InterfaceEvent>(Message::Interface);
            msg->type = Message::InterfaceEvent::MiniGameClose;
        }
    }
}

//private