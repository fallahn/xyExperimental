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
    const float minDistance = 80.f;
    const float maxDistance = 500.f;
    const float bobOffset = 200.f; //make position near his head
    const float yInfluence = 200.f;
}

RoomLightController::RoomLightController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_light         (nullptr),
    m_entity        (nullptr),
    m_intensity     (0.f)
{
    xy::Component::MessageHandler mh;
    mh.id = Message::TimeOfDay;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::TODEvent>();
        //m_light->setIntensity(1.f - data.sunIntensity);
        m_intensity = 1.f - data.sunIntensity;
    };
    addMessageHandler(mh);

    mh.id = Message::Player;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::PlayerEvent>();
        if (data.action == Message::PlayerEvent::Moved)
        {
            //if (std::abs(data.posX) > std::abs(data.posY))
            {
                sf::Vector2f position(data.posX, data.posY - bobOffset);
                static const float minDistance2 = minDistance * minDistance;
                static const float maxDistance2 = maxDistance * maxDistance;
                static const float diff = maxDistance2 - minDistance2;

                auto direction = position - m_entity->getWorldPosition();
                float distance2 = xy::Util::Vector::lengthSquared(direction);
                //artificially skew direction in Y axis so vertical lights seem further away
                distance2 *= std::max(1.f, std::abs(direction.y) / std::abs(direction.x));

                if (distance2 > maxDistance2)
                {
                    m_light->setIntensity(0.f);
                }
                else
                {
                    if (distance2 > minDistance2)
                    {
                        float playerDiff = maxDistance2 - distance2;
                        playerDiff /= diff;
                        m_light->setIntensity(m_intensity * playerDiff);
                        //REPORT("diff", std::to_string(playerDiff));
                    }
                    else
                    {
                        m_light->setIntensity(m_intensity);
                    }
                }
            }
        }
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

    m_entity = &entity;
}