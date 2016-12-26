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

#ifndef DC_MESSAGE_IDS_HPP_
#define DC_MESSAGE_IDS_HPP_

#include <xygine/MessageBus.hpp>

namespace sf
{
    class RenderWindow;
}

namespace Particle
{
    enum ID
    {
        Steam = 0x1,
        Music = 0x2,
        Sleep = 0x4
    };
}

namespace Command
{
    enum ID
    {
        Vacuum = 0x8
    };
}

namespace Message
{
    const std::int32_t CatAnimMask = 0xf00;
    enum ID
    {
        TimeOfDay = xy::Message::Count,
        DayChanged,
        NewTask,
        TaskCompleted,
        Animation,
        Particle,
        Interface,
        Attribute,
        Player,
        System
    };

    struct TODEvent final
    {
        float time = 0.f; //< TODO really got to fix this being out by 12 hours
        float sunIntensity = 0.f;
    };

    struct TaskEvent final
    {
        enum Name
        {
            Eat,
            Drink,
            Poop,
            Shower,
            Sleep,
            WatchTV,
            PlayPiano,
            PlayMusic,
            PlayComputer,
            Think,
            Travel,
            CatTask,
            Idle,
            Vacuum
        }taskName;
    };

    struct AnimationEvent final
    {
        enum ID
        {
            Up = 0,
            Down,
            Right,
            Left,
            Idle,
            Eat,
            Drink,
            Poop,
            TV,
            Piano,
            Computer,
            Sleep,
            Die,
            Scratch,
            Water,
            Feed,
            VacuumWalk,
            VacuumStill,
            Count
        }id;
    };

    struct InterfaceEvent final
    {
        enum
        {
            MouseClick,
            ButtonClick,
            MouseMoved
        }type;
        float positionX = 0.f;
        float positionY = 0.f;
        std::int16_t ID = -1;
    };

    struct AttribEvent final
    {
        enum
        {
            GotPaid,
            SpentMoney
        }action;
        std::int32_t value = 0;
    };

    struct PlayerEvent final
    {
        enum
        {
            Moved,
            TaskFailed,
            ResourceLow,
            Died
        }action;
        float posX = 0.f;
        float posY = 0.f;
        std::int32_t task = -1;
    };

    struct SystemEvent final
    {
        enum
        {
            ToggleShadowMapping,
            ResetGame
        }action;
    };
}

#endif //DC_MESSAGE_IDS_HPP_