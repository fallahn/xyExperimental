/******************************************************************

Matt Marchant 2017
http://trederia.blogspot.com

b2breakout - Zlib license.

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

#include <xygine/App.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Components/CallbackProvider.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>
#include <xygine/physics/CollisionCircleShape.hpp>

#include <xygine/imgui/imgui.h>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    sf::Uint8 input = 0;
    enum Control
    {
        Forward = 1,
        Backward = 2,
        Left = 4,
        Right = 8,
        Fire = 16
    };

    xy::Physics::RigidBody* paddleBody = nullptr;
}

Sandbox::Sandbox(xy::MessageBus& mb)
    : m_messageBus  (mb),
    m_scene         (mb),
    m_physWorld     (mb)
{
    buildArena();
    spawnPaddle();
}

Sandbox::~Sandbox()
{

}

//public
void Sandbox::update(float dt)
{
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
        case sf::Keyboard::Space:
            input |= Control::Fire;
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
        case sf::Keyboard::Space:
            input &= ~Control::Fire;
            break;
        }
    }
    else if (evt.type == sf::Event::MouseButtonReleased)
    {
        if (evt.mouseButton.button == sf::Mouse::Left)
        {
            spawnBall(xy::App::getMouseWorldPosition());
        }
    }
    else if (evt.type == sf::Event::MouseMoved)
    {
        auto pos = xy::App::getMouseWorldPosition();
        if (paddleBody)
        {
            paddleBody->setTransform({ pos.x, 900.f }, 0.f);
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

namespace
{
    const float borderThickness = 20.f;
    const sf::Vector2f blockSize(80.f, 20.f);
}

void Sandbox::buildArena()
{
    m_physWorld.setGravity({ 0.f, 0.f });
    
    auto borderBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Static);
    
    xy::Physics::CollisionRectangleShape cr({ xy::DefaultSceneSize.x, borderThickness });
    cr.setFriction(0.f);
    borderBody->addCollisionShape(cr);

    cr.setRect({ xy::DefaultSceneSize.x, borderThickness }, { 0.f, xy::DefaultSceneSize.y - borderThickness });
    borderBody->addCollisionShape(cr);

    cr.setRect({ borderThickness, xy::DefaultSceneSize.y - (borderThickness * 2.f) }, { 0.f, borderThickness });
    borderBody->addCollisionShape(cr);

    cr.setRect({ borderThickness, xy::DefaultSceneSize.y - (borderThickness * 2.f) }, { xy::DefaultSceneSize.x - borderThickness, borderThickness });
    borderBody->addCollisionShape(cr);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(borderBody);

    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    sf::Vector2f startPos(140.f, 200.f);
    for (auto y = 0; y < 8; ++y)
    {
        for (auto x = 0; x < 10; ++x)
        {
            spawnBlock({ startPos.x + ((blockSize.x + 122.f) * x), startPos.y + ((blockSize.y + 32.f) * y) });
        }
    }
}

void Sandbox::spawnBall(sf::Vector2f position)
{
    auto ballBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
    xy::Physics::CollisionCircleShape cc(20.f);
    cc.setDensity(1.f);
    cc.setRestitution(1.f);
    cc.setFriction(0.f);
    ballBody->addCollisionShape(cc);

    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(position);
    auto b = entity->addComponent(ballBody);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);

    b->applyLinearImpulse({ 100.f, -200.f }, b->getLocalCentre());
}

void Sandbox::spawnBlock(sf::Vector2f position)
{
    auto body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Static);
    xy::Physics::CollisionRectangleShape cs(blockSize);
    cs.setFriction(0.f);
    body->addCollisionShape(cs);

    auto cbp = xy::Component::create<xy::CallbackProvider>(m_messageBus);
    auto cbpPtr = cbp.get();
    xy::Component::MessageHandler mh;
    mh.id = xy::Message::PhysicsMessage;
    mh.action = [cbpPtr](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<xy::Message::PhysicsEvent>();
        if (data.event == xy::Message::PhysicsEvent::BeginContact)
        {
            if (data.contact->getCollisionShapeA()->getRigidBody()->getParentUID() == cbpPtr->getParentUID()
                /*|| data.contact->getCollisionShapeB()->getRigidBody()->getParentUID() == cbpPtr->getParentUID()*/)
            {
                cbpPtr->addUpdateCallback([](xy::Entity& entity, float) {entity.destroy(); });
            }
        }
    };
    cbp->addMessageHandler(mh);

    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(position);
    entity->addComponent(body);
    entity->addComponent(cbp);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
}

void Sandbox::spawnPaddle()
{
    auto body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Kinematic);
    xy::Physics::CollisionRectangleShape cr(blockSize, -(blockSize / 2.f));
    cr.setDensity(1.f);
    cr.setRestitution(1.f);

    body->addCollisionShape(cr);

    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(xy::DefaultSceneSize.x / 2.f, 900.f);
    paddleBody = entity->addComponent(body);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}