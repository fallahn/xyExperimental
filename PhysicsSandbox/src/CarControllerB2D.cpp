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

#include <CarControllerB2D.hpp>
#include <UserInterface.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Assert.hpp>
#include <xygine/Reports.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/physics/RigidBody.hpp>

#include <xygine/imgui/imgui.h>

namespace
{
    const sf::Vector2f forwardVector(0.f, 1.f);
    const sf::Vector2f rightVector(1.f, 0.f);

    //menu variables
    float maxForwardSpeed = 1000.f;
    float maxBackwardSpeed = -330.f;
    float driveForce = 1000.f;
    float turnSpeed = 6.f;
    float drag = 3.f;
    float density = 0.5f;
}

CarControllerB2D::CarControllerB2D(xy::MessageBus& mb, xy::Physics::RigidBody* rb, UserInterface& ui)
    : xy::Component (mb, this),
    m_body          (rb),
    m_ui            (ui),
    m_input         (0)
{
    XY_ASSERT(rb, "body can't be nullptr");
    rb->fixedRotation(false);
    

    ui.addItem([]() 
    {
        nim::InputFloat("Max Forward", &maxForwardSpeed, 10.f, 50.f);
        nim::InputFloat("Max Backward", &maxBackwardSpeed, 10.f, 50.f);
        nim::InputFloat("Drive Force", &driveForce, 10.f, 50.f);
        nim::InputFloat("Turn Speed", &turnSpeed, 10.f, 50.f);
        nim::InputFloat("Drag", &drag, 1.f, 10.f);
        nim::InputFloat("Body Density", &density, 0.1f, 1.f);
    }, this);

}

CarControllerB2D::~CarControllerB2D()
{
    m_ui.removeItems(this);
}

//public
void CarControllerB2D::entityUpdate(xy::Entity& entity, float dt)
{
    const auto& xForm = entity.getTransform();
    auto position = entity.getPosition();
    auto rightVec = xForm.transformPoint(rightVector) - position;
    auto forwardVec = xForm.transformPoint(forwardVector) - position;

    //kill lateral velocity
    sf::Vector2f impulse = m_body->getMass() * -getDirectionalVelocity(rightVec);
    m_body->applyLinearImpulse(impulse, m_body->getWorldCentre());

    //reduce angular velocity - TODO make this const a var in the menu
    m_body->applyAngularImpulse(0.1f * m_body->getInertia() * -m_body->getAngularVelocity());

    //apply drag
    m_body->applyForceToCentre(-m_body->getLinearVelocity() /** xy::Util::Vector::length(getDirectionalVelocity(forwardVec))*/ * drag);

    //---------------------------------------------//

    //update control / input
    float targetSpeed = 0.f;
    switch (m_input & (Control::Forward | Control::Backward))
    {
    case Control::Forward:
        targetSpeed = maxForwardSpeed;
        break;
    case Control::Backward:
        targetSpeed = maxBackwardSpeed;
        break;
    default: break;
    }

    if (targetSpeed != 0)
    {
        float currentSpeed = xy::Util::Vector::dot(getDirectionalVelocity(forwardVec), forwardVec);
        float force = 0.f;
        if (targetSpeed > currentSpeed)
        {
            force = driveForce;
            m_body->applyForceToCentre(force * forwardVec);
        }
        else if (targetSpeed < currentSpeed)
        {
            force = -driveForce;
            m_body->applyForceToCentre(force * forwardVec);
        }
    }

    float targetTorque = 0.f;
    switch (m_input & (Control::Left | Control::Right))
    {
    case Control::Left:
        targetTorque = turnSpeed;
        break;
    case Control::Right:
        targetTorque = -turnSpeed;
        break;
    default: break;
    }
    m_body->applyTorque(targetTorque);
    

    //m_body->getCollisionShapes()[0]->setDensity();
}

//private
sf::Vector2f CarControllerB2D::getDirectionalVelocity(const sf::Vector2f& direction) const
{
    return xy::Util::Vector::dot(direction, m_body->getLinearVelocity()) * direction;
}