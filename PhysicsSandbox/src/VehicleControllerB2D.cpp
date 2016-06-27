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

#include <VehicleControllerB2D.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Assert.hpp>
#include <xygine/Reports.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/physics/RigidBody.hpp>

#include <fstream>

void VehicleControllerB2D::Parameters::save(const std::string& path)
{
    std::ofstream file(path, std::ios::binary);
    if (file.good() && file.is_open())
    {
        file.write((char*)this, sizeof(Parameters));
    }
    else
    {
        xy::Logger::log("Failed saving car parameters " + path, xy::Logger::Type::Error);
    }
    file.close();
}

void VehicleControllerB2D::Parameters::load(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (file.good() && file.is_open())
    {
        file.read((char*)this, sizeof(Parameters));
    }
    else
    {
        xy::Logger::log("Failed reading car parameters " + path, xy::Logger::Type::Error);
    }
    file.close();
}

namespace
{
    const sf::Vector2f forwardVector(0.f, 1.f);
    const sf::Vector2f rightVector(1.f, 0.f);
}

VehicleControllerB2D::VehicleControllerB2D(xy::MessageBus& mb, xy::Physics::RigidBody* rb)
    : xy::Component (mb, this),
    m_body          (rb),
    m_input         (0),
    m_lastDensity   (0.f)
{
    XY_ASSERT(rb, "body can't be nullptr");
    rb->fixedRotation(false);
}

//public
void VehicleControllerB2D::entityUpdate(xy::Entity& entity, float dt)
{
    const auto& xForm = entity.getTransform();
    auto position = entity.getPosition();
    auto rightVec = xForm.transformPoint(rightVector) - position;
    auto forwardVec = xForm.transformPoint(forwardVector) - position;

    //kill lateral velocity
    sf::Vector2f impulse = m_body->getMass() * -getDirectionalVelocity(rightVec);
    float impulseSqr = xy::Util::Vector::lengthSquared(impulse);
    float maxImpulseSqr = m_parameters.grip * m_parameters.grip;
    if (impulseSqr > maxImpulseSqr)
    {
        impulse *= (maxImpulseSqr / impulseSqr);
    }
    m_body->applyLinearImpulse(impulse, m_body->getWorldCentre());

    //reduce angular velocity
    m_body->applyAngularImpulse(m_parameters.angularFriction * m_body->getInertia() * -m_body->getAngularVelocity());

    //apply drag
    m_body->applyForceToCentre(-m_body->getLinearVelocity() /** xy::Util::Vector::length(getDirectionalVelocity(forwardVec))*/ * m_parameters.drag);

    //---------------------------------------------//

    //update control / input
    float targetSpeed = 0.f;
    switch (m_input & (Control::Forward | Control::Backward))
    {
    case Control::Forward:
        targetSpeed = m_parameters.maxForwardSpeed;
        break;
    case Control::Backward:
        targetSpeed = m_parameters.maxBackwardSpeed;
        break;
    default: break;
    }

    float currentSpeed = xy::Util::Vector::dot(getDirectionalVelocity(forwardVec), forwardVec);
    if (targetSpeed != 0)
    {        
        float force = 0.f;
        if (targetSpeed > currentSpeed)
        {
            force = m_parameters.driveForce;
            m_body->applyForceToCentre(force * forwardVec);
        }
        else if (targetSpeed < currentSpeed)
        {
            force = -m_parameters.driveForce;
            m_body->applyForceToCentre(force * forwardVec);
        }
    }

    float targetTorque = 0.f;
    switch (m_input & (Control::Left | Control::Right))
    {
    case Control::Left:
        targetTorque = m_parameters.turnSpeed * currentSpeed;
        break;
    case Control::Right:
        targetTorque = -m_parameters.turnSpeed * currentSpeed;
        break;
    default: break;
    }
    m_body->applyTorque(targetTorque);
}

void VehicleControllerB2D::setParameters(const Parameters& p)
{
    m_parameters = p;
    if (p.density != m_lastDensity)
    {
        const auto& shapes = m_body->getCollisionShapes();
        if (!shapes.empty())
        {
            shapes[0]->setDensity(p.density);
            //we're assuming there's a second body somewhere to
            //create a similar influence to being front or rear
            //wheel drive
            if (shapes.size() > 1)
            {
                shapes[1]->setDensity(p.density * 4.2f);
            }
        }
    }
    m_lastDensity = p.density;
}

//private
sf::Vector2f VehicleControllerB2D::getDirectionalVelocity(const sf::Vector2f& direction) const
{
    return xy::Util::Vector::dot(direction, m_body->getLinearVelocity()) * direction;
}