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
#include <xygine/util/Vector.hpp>
#include <xygine/Reports.hpp>

namespace
{
    const float bobOffset = 180.f; //make position near his head

    float intensityMultiplier = 6.f; //this is used to increase brightness of lights when shadow maps are enabled

    const sf::Vector2f radius(400.f, 128.f);
    bool contains(sf::Vector2f point, sf::Vector2f ellipse)
    {
        float sX = point.x / ellipse.x;
        float sY = point.y / ellipse.y;
        return ((sX * sX) + (sY * sY)) < 1.f;
    };
}

RoomLightController::RoomLightController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_light         (nullptr),
    m_entity        (nullptr),
    m_up            (false),
    m_fade          (0.f),
    m_intensity     (0.f)
{
    //change intensity with day time
    xy::Component::MessageHandler mh;
    mh.id = Message::TimeOfDay;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::TODEvent>();
        m_intensity = (1.f - data.sunIntensity) * intensityMultiplier;
    };
    addMessageHandler(mh);

    //change intensity with proximity of player
    mh.id = Message::Player;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::PlayerEvent>();
        if (data.action == Message::PlayerEvent::Moved)
        {
            m_up = contains(sf::Vector2f(data.posX, data.posY - bobOffset) - m_entity->getWorldPosition(), radius);
        }
    };
    addMessageHandler(mh);

    //toggle shadow mapping if requested
    mh.id = Message::System;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::SystemEvent>();
        if (data.action == Message::SystemEvent::ToggleShadowMapping)
        {
            toggleShadowMap(data.value);
        }
    };
    addMessageHandler(mh);
}

//public
void RoomLightController::entityUpdate(xy::Entity& entity, float dt)
{
    //REPORT("light intens", std::to_string(m_light->getIntensity()));
    dt *= 2.f;
    if (m_up)
    {
        m_fade = std::min(1.f, m_fade + dt);
    }
    else
    {
        m_fade = std::max(0.f, m_fade - dt);
    }
    m_light->setIntensity(m_intensity * m_fade);
}

void RoomLightController::onStart(xy::Entity& entity)
{
    m_light = entity.getComponent<xy::PointLight>();
    XY_ASSERT(m_light, "Entity has no light component");

    m_entity = &entity;
}

void RoomLightController::toggleShadowMap(bool v)
{
    XY_ASSERT(m_light, "controller not init yet");
    if (v)
    {
        intensityMultiplier = 9.f;
        m_light->enableShadowCasting(true);
    }
    else
    {
        intensityMultiplier = 1.f;
        m_light->enableShadowCasting(false);
    }
}