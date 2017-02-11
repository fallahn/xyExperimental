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
#include <MGDarts.hpp>
#include <MGPachinko.hpp>
#include <MiniGameIDs.hpp>

#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/components/AudioSource.hpp>
#include <xygine/util/Const.hpp>
#include <xygine/util/Position.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/physics/JointHinge.hpp>
#include <xygine/physics/CollisionPolygonShape.hpp>
#include <xygine/physics/CollisionEdgeShape.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>

#include <xygine/shaders/Default.hpp>

namespace
{
    const sf::Vector2f gamePosition(xy::DefaultSceneSize.x / 2.f, 420.f);
    const float rouletteRadius = 260.f;
}

void WorldClientState::createRoulette()
{
    auto staticBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Static);
    auto gameController = xy::Component::create<RouletteGame>(m_messageBus, m_textureResource, m_scene, m_attribManager);
    //gameController->setShader(&m_shaderResource.get(xy::Shader::Count));

    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(gamePosition);
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
    sensorFilter.categoryFlags = Roulette::Segment;
    sensorFilter.maskFlags = ~Roulette::Ball;
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

    xy::Physics::HingeJoint hj(*bPtr, gamePosition);
    rotatingBody->addJoint(hj);

    auto wheelDrb = xy::Component::create<xy::SfDrawableComponent<sf::Sprite>>(m_messageBus);
    wheelDrb->getDrawable().setTexture(m_textureResource.get("assets/images/minigames/roulette/wheel.png"));
    //wheelDrb->setShader(&m_shaderResource.get(xy::Shader::Count));
    xy::Util::Position::centreOrigin(wheelDrb->getDrawable());

    auto wheelSound = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
    wheelSound->setSound("assets/sound/minigame/wheel_click.wav");
    wheelSound->setAttenuation(0.01f);
    wheelSound->setFadeOutTime(0.2f);
    
    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(gamePosition);
    entity->addCommandCategories(Command::ID::MiniGame | Command::ID::RouletteWheel);
    entity->addComponent(rotatingBody);
    entity->addComponent(wheelDrb);
    entity->addComponent(wheelSound);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);


    //ball
    auto ballBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
    cs.setRadius(14.f);
    cs.setRestitution(0.4f);
    cs.setFriction(2.1f);
    cs.setDensity(2.f);
    cs.setUserID(Roulette::Ball);
    ballBody->addCollisionShape(cs);
    ballBody->isBullet(true);

    auto ballDwb = xy::Component::create<xy::SfDrawableComponent<sf::Sprite>>(m_messageBus);
    ballDwb->getDrawable().setTexture(m_textureResource.get("assets/images/minigames/roulette/ball.png"));
    //ballDwb->setShader(&m_shaderResource.get(xy::Shader::Count));
    xy::Util::Position::centreOrigin(ballDwb->getDrawable());

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(gamePosition);
    entity->move(0.f, 240.f);
    entity->addCommandCategories(Command::ID::MiniGame | Command::ID::RouletteBall);
    entity->addComponent(ballBody);
    entity->addComponent(ballDwb);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void WorldClientState::createDarts()
{
    auto dartsController = xy::Component::create<DartsGame>(m_messageBus, m_textureResource, m_attribManager);

    auto wheelSound = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
    wheelSound->setSound("assets/sound/minigame/wheel_click.wav");
    wheelSound->setAttenuation(0.01f);
    wheelSound->setFadeOutTime(0.2f);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addCommandCategories(Command::ID::MiniGame);
    entity->addComponent(dartsController);
    entity->addComponent(wheelSound);
    entity->setPosition(gamePosition);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void WorldClientState::createPachinko()
{
    auto fixedBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Static);

    //edge shape
    std::vector<sf::Vector2f> points = 
    {
        sf::Vector2f(176.f, 32.f),
        {192.f, 0.f},
        {256.f, 32.f},
        {288.f, 64.f},
        {352.f, 192.f},
        {352.f, 416.f},
        {336.f, 416.f},
        {192.f, 512.f},
        {160.f, 512.f},
        {0.f, 416.f},
        {0.f, 192.f},
        {64.f, 64.f},
        {96.f, 32.f},
        {160.f, 0.f}
    };
    sf::Vector2f offset(176.f, 256.f);
    for (auto& p : points) p -= offset;
    xy::Physics::CollisionEdgeShape es(points, xy::Physics::CollisionEdgeShape::Option::Loop);
    es.setRestitution(0.01f);
    fixedBody->addCollisionShape(es);

    //start edge
    xy::Physics::CollisionRectangleShape rs({ 6.f, 208.f }, sf::Vector2f(326.f, 208.f) - offset);
    rs.setDensity(1.f);
    rs.setFriction(0.05f);
    rs.setRestitution(0.02f);
    fixedBody->addCollisionShape(rs);

    //pins
    points =
    {
        sf::Vector2f(112.f, 80.f),
        {144.f, 80.f},
        {176.f, 80.f},
        {208.f, 80.f},
        {240.f, 80.f},

        {96.f, 112.f},
        {128.f, 112.f},
        {160.f, 112.f},
        {192.f, 112.f},
        {224.f, 112.f},
        {256.f, 112.f},

        {112.f, 80.f},
        {144.f, 80.f},
        {176.f, 80.f},
        {208.f, 80.f},
        {240.f, 80.f},

        {128.f, 176.f},
        {224.f, 176.f},
        {176.f, 208.f},
        {64.f, 224.f},
        {288.f, 224.f},
        {112.f, 256.f},
        {240.f, 256.f},

        {80.f, 336.f},
        {112.f, 336.f},
        {144.f, 336.f},
        {176.f, 336.f},
        {208.f, 336.f},
        {240.f, 336.f},
        {272.f, 336.f},

        {96.f, 368.f},
        {128.f, 368.f},
        {160.f, 368.f},
        {192.f, 368.f},
        {224.f, 368.f},
        {256.f, 368.f},

        {80.f, 400.f},
        {176.f, 400.f},
        {272.f, 400.f}
    };

    for (const auto& p : points)
    {
        xy::Physics::CollisionCircleShape cs(8.f);
        cs.setPosition(p - offset);
        cs.setDensity(1.f);
        cs.setRestitution(0.1f);
        fixedBody->addCollisionShape(cs);
    }

    //bouncers
    xy::Physics::CollisionCircleShape cs(24.f);
    cs.setDensity(0.2f);
    cs.setRestitution(1.f);
    cs.setUserID(Pachinko::Bouncer); //used to find collision and apply ball force
    cs.setPosition(sf::Vector2f(80.f, 288.f) - offset);
    fixedBody->addCollisionShape(cs);
    cs.setPosition(sf::Vector2f(272.f, 288.f) - offset);
    fixedBody->addCollisionShape(cs);

    //win 2 bucket
    auto addBucket = [&](sf::Vector2f pos, int id)
    {
        xy::Physics::CollisionCircleShape sensor(5.f);
        sensor.setIsSensor(true);
        sensor.setUserID(id);
        sensor.setPosition(pos);
        xy::Physics::CollisionFilter cf;
        cf.maskFlags = Roulette::Ball;
        sensor.setFilter(cf);
        fixedBody->addCollisionShape(sensor);

        std::vector<sf::Vector2f> edgePoints = 
        {
            sf::Vector2f(-20.f, -20.f) + pos,
            sf::Vector2f(-12.f, 0.f) + pos,
            sf::Vector2f(0.f, 6.f) + pos,
            sf::Vector2f(12.f, 0.f) + pos,
            sf::Vector2f(20.f, -20.f) + pos
        };
        xy::Physics::CollisionEdgeShape bucket(edgePoints);
        fixedBody->addCollisionShape(bucket);
    };
    addBucket({ 0.f, 26.f }, Pachinko::WinTwoBucket);

    //win 1 buckets
    addBucket({ 48.f, 166.f }, Pachinko::WinOneBucket);
    addBucket({ -48.f, 166.f }, Pachinko::WinOneBucket);

    //loser hole
    cs.setRadius(16.f);
    cs.setIsSensor(true);
    xy::Physics::CollisionFilter cf;
    cf.maskFlags = Roulette::Ball;
    cs.setFilter(cf);
    cs.setPosition({ 0.f, 256.f });
    cs.setUserID(Pachinko::LoserHole);
    fixedBody->addCollisionShape(cs);

    //ball launcher
    xy::Physics::CollisionRectangleShape launchShape({ 16.f, 36.f }, { 160.f, 124.f });
    launchShape.setIsSensor(true);
    launchShape.setUserID(Pachinko::LaunchSpring);
    launchShape.setFilter(cf);
    fixedBody->addCollisionShape(launchShape);

    auto dwb = xy::Component::create<xy::SfDrawableComponent<sf::Sprite>>(m_messageBus);
    dwb->getDrawable().setTexture(m_textureResource.get("assets/images/minigames/pachinko/table.png"));
    xy::Util::Position::centreOrigin(dwb->getDrawable());

    auto controller = xy::Component::create<PachinkoGame>(m_messageBus, m_scene, m_textureResource);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addCommandCategories(Command::ID::MiniGame);
    entity->setPosition(/*gamePosition*/(xy::DefaultSceneSize / 2.f) + sf::Vector2f(0.f, -100.f));
    entity->addComponent(fixedBody);
    entity->addComponent(dwb);
    entity->addComponent(controller);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}