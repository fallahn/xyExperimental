/*********************************************************************
Matt Marchant 2017
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

#include <MGPachinko.hpp>
#include <MiniGameIDs.hpp>
#include <MessageIDs.hpp>

#include <xygine/Scene.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Resource.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionCircleShape.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace
{
    const sf::Vector2f ballSpawn = sf::Vector2f(162.f, -36.f) + (xy::DefaultSceneSize / 2.f);
    const float maxPower = 35.f;
    const float minPower = 20.f;
}

PachinkoGame::PachinkoGame(xy::MessageBus& mb, xy::Scene& scene, xy::TextureResource& tr)
    : xy::Component (mb, this),
    m_scene         (scene),
    m_currentState  (State::Purchase),
    m_powerbar      (tr.get("assets/images/minigames/pachinko/powerbar.png"))
{
    m_powerbar.rotate(-90.f);
    m_powerbar.setPosition(300.f, 100.f);
    
    spawnBall();

    xy::Component::MessageHandler handler;
    handler.id = Message::Interface;
    handler.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::InterfaceEvent>();
        if (data.type == Message::InterfaceEvent::KeyPress)
        {
            switch (data.ID)
            {
            default: break;
            case sf::Keyboard::Space:
            {
                switch (m_currentState)
                {
                default: break;
                case State::Purchase:

                    m_currentState = State::Charge;
                    break;
                case State::Charge:
                {
                    float value = (m_powerbar.getValue() + 1.f) / 2.f;
                    value *= (maxPower - minPower) + minPower;

                    xy::Command cmd;
                    cmd.category = Command::ID::RouletteBall;
                    cmd.action = [value](xy::Entity& entity, float)
                    {
                        entity.getComponent<xy::Physics::RigidBody>()->applyLinearImpulse({ 0.f, -value }, {});
                    };
                    m_scene.sendCommand(cmd);

                    m_currentState = State::Play;
                    break;
                }
                }
            }
                break;
            }
        }
    };
    addMessageHandler(handler);

}

//public
void PachinkoGame::entityUpdate(xy::Entity& entity, float dt)
{
    switch (m_currentState)
    {
    default: break;
    case State::Purchase:

        break;
    case State::Charge:
        m_powerbar.update(dt);
        break;
    case State::Play:

        break;
    case State::Summary:

        break;
    case State::GameOver:

        break;
    }
}


//private
void PachinkoGame::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_powerbar, states);
}

void PachinkoGame::spawnBall()
{
    auto body = xy::Component::create<xy::Physics::RigidBody>(getMessageBus(), xy::Physics::BodyType::Dynamic);
    xy::Physics::CollisionCircleShape cs(7.f);
    cs.setDensity(0.8f);
    cs.setRestitution(0.01f);
    xy::Physics::CollisionFilter cf;
    cf.categoryFlags = Roulette::Ball;
    cs.setFilter(cf);
    body->addCollisionShape(cs);
    body->isBullet(true);

    auto entity = xy::Entity::create(getMessageBus());
    entity->setPosition(ballSpawn);
    entity->addCommandCategories(Command::ID::MiniGame);
    auto bPtr = entity->addComponent(body);
    auto ePtr = m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);

    xy::Component::MessageHandler handler;
    handler.id = xy::Message::PhysicsMessage;
    handler.action = [ePtr, bPtr, this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<xy::Message::PhysicsEvent>();
        if (data.event == xy::Message::PhysicsEvent::BeginContact)
        {            
            std::int32_t uid = -1;
            if(data.contact->getCollisionShapeA()->getRigidBody() == bPtr)
            {
                uid = data.contact->getCollisionShapeB()->getUserID();
            }
            else if (data.contact->getCollisionShapeB()->getRigidBody() == bPtr)
            {
                uid = data.contact->getCollisionShapeA()->getUserID();
            }

            switch (uid)
            {
            default:
                std::cout << uid << std::endl;
                break;
            case Pachinko::LaunchSpring:
                ePtr->addCommandCategories(Command::ID::RouletteBall);
                std::cout << " cat added" << std::endl;
                break;
            case Pachinko::Bouncer:
                spawnBall();
                std::cout << "spoing!" << std::endl;
                break;
            case Pachinko::LoserHole:
                ePtr->destroy();
                spawnBall();
                std::cout << "in hole" << std::endl;
                break;
            case Pachinko::WinOneBucket:
                ePtr->destroy();
                spawnBall();
                break;
            case Pachinko::WinTwoBucket:
                ePtr->destroy();
                spawnBall();
                break;
            }
        }
        else if (data.event == xy::Message::PhysicsEvent::EndContact)
        {
            if ((data.contact->getCollisionShapeA()->getUserID() == Pachinko::LaunchSpring &&
                data.contact->getCollisionShapeB()->getRigidBody() == bPtr) ||
                (data.contact->getCollisionShapeB()->getUserID() == Pachinko::LaunchSpring &&
                    data.contact->getCollisionShapeA()->getRigidBody() == bPtr))
            {
                ePtr->removeCommandCategories(Command::ID::RouletteBall);
                std::cout << " cat removed" << std::endl;
            }
        }
    };
    bPtr->addMessageHandler(handler);
}