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

#include <RoomLightController.hpp>
#include <MessageIDs.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Assert.hpp>
#include <xygine/components/PointLight.hpp>

RoomLightController::RoomLightController(xy::MessageBus& mb)
    : xy::Component(mb, this),
    m_light(nullptr),
    m_intensity(0.f)
{
    xy::Component::MessageHandler mh;
    mh.id = Message::TimeOfDay;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::TODEvent>();
        m_light->setIntensity(1.f - data.sunIntensity);
    };
    addMessageHandler(mh);
}

//public
void RoomLightController::entityUpdate(xy::Entity& entity, float)
{

}

void RoomLightController::onStart(xy::Entity& entity)
{
    m_light = entity.getComponent<xy::PointLight>();
    XY_ASSERT(m_light, "Entity has no light component");
}