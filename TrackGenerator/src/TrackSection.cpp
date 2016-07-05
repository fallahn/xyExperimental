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

#include <TrackSection.hpp>

#include <xygine/Assert.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionEdgeShape.hpp>

#include <array>
#include <vector>

namespace
{
    /*
    Track sections are 1024x1024 in size.
    A connection section is 300x256
    */

    //set up some consts for these values
    const float sectionSize = 1024.f;
    const float connectionWidth = 300;
    const float connectionHeight = 256.f;
    const float connectionBend = 200.f;
    const float connectionGap = 62.f;

    struct Connection final
    {
        std::vector<sf::Vector2f> leftEdge;
        std::vector<sf::Vector2f> rightEdge;
    };
    //left to right, top to bottom
    std::array<Connection, 6u> connections;
    bool created = false;
}

TrackSection::TrackSection()
{
    if (!created) //TODO we might get better performance if we directly cache the collision edges
    {
        created = true;
        connections[0].leftEdge = { sf::Vector2f(), sf::Vector2f(0.f, 256.f) };
        connections[0].rightEdge = { sf::Vector2f(300.f, 0.f), sf::Vector2f(300.f, 200.f), sf::Vector2f(362.f, 256.f) };

        connections[1].leftEdge = { sf::Vector2f(362.f, 0.f), sf::Vector2f(362.f, 256.f) };
        connections[1].rightEdge = { sf::Vector2f(662.f, 0.f), sf::Vector2f(662.f, 256.f) };

        connections[2].leftEdge = { sf::Vector2f(662.f, 256.f), sf::Vector2f(724.f, 200.f), sf::Vector2f(724.f, 0.f) };
        connections[2].rightEdge = { sf::Vector2f(1024.f, 0.f), sf::Vector2f(1024.f, 256.f) };

        connections[3].leftEdge = { sf::Vector2f(0.f, 768.f), sf::Vector2f(0.f, 1024.f) };
        connections[3].rightEdge = { sf::Vector2f(362.f, 768.f), sf::Vector2f(300.f, 824.f), sf::Vector2f(300.f, 1024.f) };

        connections[4].leftEdge = { sf::Vector2f(362.f, 768.f), sf::Vector2f(362.f, 1024.f) };
        connections[4].rightEdge = { sf::Vector2f(662.f, 768.f), sf::Vector2f(662.f, 1024.f) };

        connections[5].leftEdge = { sf::Vector2f(662.f, 768.f), sf::Vector2f(724.f, 824.f), sf::Vector2f(724.f, 1024.f) };
        connections[5].rightEdge = { sf::Vector2f(1024.f, 768.f), sf::Vector2f(1024.f, 1024.f) };
    }
}

//public
xy::Entity::Ptr TrackSection::create(sf::Uint16 uid, xy::MessageBus& mb)
{   
    auto body = xy::Component::create<xy::Physics::RigidBody>(mb, xy::Physics::BodyType::Kinematic);
    
    //top two points of centre part
    sf::Vector2f tl(sectionSize, connectionHeight);
    sf::Vector2f tr(0.f, connectionHeight);

    //lower half of ID represents top 3 connections
    auto bits = uid & 0x0f;
    XY_ASSERT(bits != 0 && bits != 0x8, "can't have empty segments");
    if (bits & 0x4)
    {
        //top left
        xy::Physics::CollisionEdgeShape es(connections[0].leftEdge);
        body->addCollisionShape(es);
        es.setPoints(connections[0].rightEdge);
        body->addCollisionShape(es);

        if (tl.x > 0.f) tl.x = 0.f;
        if (tr.x < connectionWidth + connectionGap) tr.x = connectionWidth + connectionGap;
    }
    if (bits & 0x2)
    {
        //top middle
        xy::Physics::CollisionEdgeShape es(connections[1].leftEdge);
        body->addCollisionShape(es);
        es.setPoints(connections[1].rightEdge);
        body->addCollisionShape(es);

        if (tl.x > connectionWidth + connectionGap) tl.x = connectionWidth + connectionGap;
        if (tr.x < (connectionWidth * 2.f) + connectionGap) tr.x = (connectionWidth * 2.f) + connectionGap;
    }
    if (bits & 0x1)
    {
        //top right
        xy::Physics::CollisionEdgeShape es(connections[2].leftEdge);
        body->addCollisionShape(es);
        es.setPoints(connections[2].rightEdge);
        body->addCollisionShape(es);

        if (tl.x >(connectionWidth * 2.f) + connectionGap) tl.x = (connectionWidth * 2.f) + connectionGap;
        if (tr.x < sectionSize) tr.x = sectionSize;
    }

    //bottom two points of centre part
    sf::Vector2f bl(sectionSize, connectionHeight * 3.f);
    sf::Vector2f br(0.f, connectionHeight * 3.f);

    //else bottom 3
    bits = ((uid & 0xf0) >> 4);
    XY_ASSERT(bits != 0 && bits != 0x8, "can't have empty segments");

    if (bits & 0x4)
    {
        //bottom left
        xy::Physics::CollisionEdgeShape es(connections[3].leftEdge);
        body->addCollisionShape(es);
        es.setPoints(connections[3].rightEdge);
        body->addCollisionShape(es);

        if (bl.x > 0.f) bl.x = 0.f;
        if (br.x < connectionWidth + connectionGap) br.x = connectionWidth + connectionGap;
    }
    if (bits & 0x2)
    {
        //bottom middle
        xy::Physics::CollisionEdgeShape es(connections[4].leftEdge);
        body->addCollisionShape(es);
        es.setPoints(connections[4].rightEdge);
        body->addCollisionShape(es);

        if (bl.x > connectionWidth + connectionGap) bl.x = connectionWidth + connectionGap;
        if (br.x < (connectionWidth * 2.f) + connectionGap) br.x = (connectionWidth * 2.f) + connectionGap;
    }
    if (bits & 0x1)
    {
        //bottom right
        xy::Physics::CollisionEdgeShape es(connections[5].leftEdge);
        body->addCollisionShape(es);
        es.setPoints(connections[5].rightEdge);
        body->addCollisionShape(es);

        if (bl.x >(connectionWidth * 2.f) + connectionGap) bl.x = (connectionWidth * 2.f) + connectionGap;
        if (br.x < sectionSize) br.x = sectionSize;
    }

    //create centre part
    xy::Physics::CollisionEdgeShape es({ tl, bl });
    body->addCollisionShape(es);
    es.setPoints({ tr, br });
    body->addCollisionShape(es);

    auto entity = xy::Entity::create(mb);
    entity->addComponent(body);
    return std::move(entity);
}