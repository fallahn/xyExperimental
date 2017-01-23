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

#include <WorldClientState.hpp>
#include <MGRoulette.hpp>

#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/util/Const.hpp>
#include <xygine/util/Position.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/physics/JointHinge.hpp>
#include <xygine/physics/CollisionPolygonShape.hpp>

namespace
{
    const sf::Vector2f roulettePosition(xy::DefaultSceneSize.x / 2.f, 420.f);
    const float rouletteRadius = 260.f;

    enum CollisionID
    {
        Ball    = 0x10,
        Segment = 0x20
    };
}

void WorldClientState::createRoulette()
{
    auto staticBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Static);
    auto gameController = xy::Component::create<RouletteGame>(m_messageBus, m_textureResource, m_scene, m_attribManager);

    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(roulettePosition);
    entity->addCommandCategories(Command::ID::MiniGame);
    auto bPtr = entity->addComponent(staticBody);
    entity->addComponent(gameController);

    m_scene.addEntity(entity, xy::Scene::Layer::UI);

    auto rotatingBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
    rotatingBody->setAngularDamping(0.45f);
    auto angle = xy::Util::Const::TAU / 7.f;

    std::vector<sf::Vector2f> points(7);
    for (auto i = 0u; i < 7u; ++i)
    {
        auto theta = angle * i;
        points[i].x = std::sin(theta);
        points[i].y = std::cos(theta);
        points[i] *= rouletteRadius;
    }

    xy::Physics::CollisionFilter sensorFilter;
    sensorFilter.categoryFlags = CollisionID::Segment;
    sensorFilter.maskFlags = ~CollisionID::Ball;
    for (auto i = 0; i < 7; ++i)
    {
        auto nextPoint = points[(i + 1) % points.size()];
        std::vector<sf::Vector2f> poly =
        {
            points[i],
            nextPoint
        };
        poly.push_back(poly[0] - poly[1]);
        float temp = poly[2].y;
        poly[2].y = -poly[2].x;
        poly[2].x = temp;
        poly[2] *= 1.1f;

        xy::Physics::CollisionPolygonShape ps(poly);
        ps.setDensity(0.8f);
        ps.setRestitution(0.4f);
        ps.setFriction(0.05f);
        ps.setIsSensor(false);
        rotatingBody->addCollisionShape(ps);

        poly[0] = points[i] + ((nextPoint - points[i]) / 2.f);
        poly[1] = nextPoint;
        poly[2] = nextPoint + ((points[(i + 2) % points.size()] - nextPoint) / 2.f);
        poly.emplace_back();
        ps.setPoints(poly);
        ps.setDensity(0.005f);
        ps.setIsSensor(true);
        ps.setFilter(sensorFilter);
        ps.setUserID(i);
        rotatingBody->addCollisionShape(ps);
    }

    //needs a shape for the pivot / friction
    xy::Physics::CollisionCircleShape cs(80.f);
    cs.setRestitution(0.2f);
    cs.setDensity(0.5f);
    cs.setFriction(0.25f);
    rotatingBody->addCollisionShape(cs);

    xy::Physics::HingeJoint hj(*bPtr, roulettePosition);
    rotatingBody->addJoint(hj);

    auto wheelDrb = xy::Component::create<xy::SfDrawableComponent<sf::Sprite>>(m_messageBus);
    wheelDrb->getDrawable().setTexture(m_textureResource.get("assets/images/minigames/roulette/wheel.png"));
    xy::Util::Position::centreOrigin(wheelDrb->getDrawable());

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(roulettePosition);
    entity->addCommandCategories(Command::ID::MiniGame | Command::ID::RouletteWheel);
    entity->addComponent(rotatingBody);
    entity->addComponent(wheelDrb);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);


    //ball
    auto ballBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
    cs.setRadius(14.f);
    cs.setRestitution(0.4f);
    cs.setFriction(2.1f);
    cs.setDensity(2.f);
    ballBody->addCollisionShape(cs);
    ballBody->isBullet(true);

    auto ballDwb = xy::Component::create<xy::SfDrawableComponent<sf::Sprite>>(m_messageBus);
    ballDwb->getDrawable().setTexture(m_textureResource.get("assets/images/minigames/roulette/ball.png"));
    xy::Util::Position::centreOrigin(ballDwb->getDrawable());

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(roulettePosition);
    entity->move(0.f, 240.f);
    entity->addCommandCategories(Command::ID::MiniGame | Command::ID::RouletteBall);
    entity->addComponent(ballBody);
    entity->addComponent(ballDwb);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}