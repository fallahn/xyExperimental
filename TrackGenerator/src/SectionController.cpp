/******************************************************************

Matt Marchant 2016
http://trederia.blogspot.com

xyRacer - Zlib license.

This software is provided 'as-is', without any express or
implied warranty.In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions :

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.

******************************************************************/

#include <SectionController.hpp>
#include <TrackSection.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/physics/RigidBody.hpp>

SectionController::SectionController(xy::MessageBus& mb, TrackSection& ts)
    : xy::Component (mb, this),
    m_trackSection  (ts)
{
    //LOG("ent created", xy::Logger::Type::Info);
}

//public
void SectionController::entityUpdate(xy::Entity& entity, float dt)
{
    if (entity.getPosition().y > xy::DefaultSceneSize.y)
    {
        entity.destroy();
        auto newSection = m_trackSection.create(getMessageBus(), entity.getPosition().y - (2.f * TrackSection::getSectionSize()));
        entity.getScene()->addEntity(newSection, xy::Scene::Layer::FrontRear);
        //LOG("ent destroyed", xy::Logger::Type::Info);
    }

    //speed up over time
    auto rb = entity.getComponent<xy::Physics::RigidBody>();
    rb->setLinearVelocity({ 0.f, rb->getLinearVelocity().y + (dt * TrackSection::getSpeedIncrease()) });
}