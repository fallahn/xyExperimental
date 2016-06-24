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

#include <Sandbox.hpp>
#include <UserInterface.hpp>
#include <CarControllerB2D.hpp>

#include <xygine/MessageBus.hpp>
#include <xygine/Entity.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>
#include <xygine/physics/JointFriction.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    CarControllerB2D* controller = nullptr;
    sf::Uint8 input = 0;
}

Sandbox::Sandbox(xy::MessageBus& mb, UserInterface& ui)
    : m_messageBus  (mb),
    m_ui            (ui),
    m_scene         (mb),
    m_physWorld     (mb)
{
    setupVehicles();
}

Sandbox::~Sandbox()
{
    m_ui.removeItems(this);
}

//public
void Sandbox::update(float dt)
{
    controller->setInput(input);
    m_scene.update(dt);
}

void Sandbox::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyPressed)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::W:
            input |= Control::Forward;
            break;
        case sf::Keyboard::A:
            input |= Control::Left;
            break;
        case sf::Keyboard::S:
            input |= Control::Backward;
            break;
        case sf::Keyboard::D:
            input |= Control::Right;
            break;
        }
    }
    else if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::W:
            input &= ~Control::Forward;
            break;
        case sf::Keyboard::A:
            input &= ~Control::Left;
            break;
        case sf::Keyboard::S:
            input &= ~Control::Backward;
            break;
        case sf::Keyboard::D:
            input &= ~Control::Right;
            break;
        }
    }
}

void Sandbox::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
}

void Sandbox::setView(const sf::View& v)
{
    m_scene.setView(v);
    m_view = v;
}

//private
void Sandbox::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_scene);
    rt.setView(m_view);
    rt.draw(m_physWorld);
}

void Sandbox::setupVehicles()
{
    m_physWorld.setGravity({ 0.f, 0.f });
    
    auto body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Static);
    auto shape = xy::Physics::CollisionRectangleShape({ 10.f, 576.f });
    body->addCollisionShape(shape);
    shape.setRect({ 10.f, 576.f }, { 1014.f, 0.f });
    body->addCollisionShape(shape);
    shape.setRect({ 1004.f, 10.f }, { 10.f, 0.f });
    body->addCollisionShape(shape);
    shape.setRect({ 1004.f, 10.f }, { 10.f, 566.f });
    body->addCollisionShape(shape);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(body);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    //vehicle
    body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
    shape.setRect({ 25.f, 64.f }, { -12.5f, -32.f });
    shape.setDensity(0.5f);
    body->addCollisionShape(shape);

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(512.f, 288.f);
    auto bodyPtr = entity->addComponent(body);

    auto carController = xy::Component::create<CarControllerB2D>(m_messageBus, bodyPtr, m_ui);
    controller = entity->addComponent(carController);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
}