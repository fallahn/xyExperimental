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
#include <SectionController.hpp>

#include <xygine/Assert.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Reports.hpp>

#include <array>
#include <vector>

namespace
{
    /*
    Track sections are 1080x1080 in size.
    A connection section is 340x270
    */

    //set up some consts for these values
    const float sectionSize = xy::DefaultSceneSize.y;
    const float connectionWidth = 340.f;
    const float connectionHeight = sectionSize / 4.f;
    const float connectionBend = 200.f;
    const float connectionGap = (sectionSize - (connectionWidth * 3.f)) / 2.f;

    const float speedIncrease = 5.f;

    struct Connection final
    {
        std::vector<sf::Vector2f> leftEdge;
        std::vector<sf::Vector2f> rightEdge;
    };
    //left to right, top to bottom
    std::array<Connection, 6u> connections; //TODO remove this once edges cached
    bool created = false;
}

TrackSection::TrackSection()
    : m_index           (0u),
    m_initialVelocity   (600.f)
{
    if (!created)
    {
        created = true;
        connections[0].leftEdge = { sf::Vector2f(), sf::Vector2f(0.f, connectionHeight) };
        connections[0].rightEdge = { sf::Vector2f(connectionWidth, 0.f), sf::Vector2f(connectionWidth, connectionBend), sf::Vector2f(connectionWidth + connectionGap, connectionHeight) };

        connections[1].leftEdge = { sf::Vector2f(connectionWidth + connectionGap, 0.f), sf::Vector2f(connectionWidth + connectionGap, connectionHeight) };
        connections[1].rightEdge = { sf::Vector2f((connectionWidth * 2.f) + connectionGap, 0.f), sf::Vector2f((connectionWidth * 2.f) + connectionGap, connectionHeight) };

        connections[2].leftEdge = { sf::Vector2f((connectionWidth * 2.f) + connectionGap, connectionHeight), sf::Vector2f(sectionSize - connectionWidth, connectionBend), sf::Vector2f(sectionSize - connectionWidth, 0.f) };
        connections[2].rightEdge = { sf::Vector2f(sectionSize, 0.f), sf::Vector2f(sectionSize, connectionHeight) };

        connections[3].leftEdge = { sf::Vector2f(0.f, sectionSize - connectionHeight), sf::Vector2f(0.f, sectionSize) };
        connections[3].rightEdge = { sf::Vector2f(connectionWidth + connectionGap, sectionSize - connectionHeight), sf::Vector2f(connectionWidth, sectionSize - connectionBend), sf::Vector2f(connectionWidth, sectionSize) };

        connections[4].leftEdge = { sf::Vector2f(connectionWidth + connectionGap, sectionSize - connectionHeight), sf::Vector2f(connectionWidth + connectionGap, sectionSize) };
        connections[4].rightEdge = { sf::Vector2f((connectionWidth * 2.f) + connectionGap, sectionSize - connectionHeight), sf::Vector2f((connectionWidth * 2.f) + connectionGap, sectionSize) };

        connections[5].leftEdge = { sf::Vector2f((connectionWidth * 2.f) + connectionGap, sectionSize - connectionHeight), sf::Vector2f(sectionSize - connectionWidth, sectionSize - connectionBend), sf::Vector2f(sectionSize - connectionWidth, sectionSize) };
        connections[5].rightEdge = { sf::Vector2f(sectionSize, sectionSize - connectionHeight), sf::Vector2f(sectionSize, sectionSize) };
    }

    EdgeCollection ec;
    ec.top = 
    {
        xy::Physics::CollisionEdgeShape({ sf::Vector2f(), sf::Vector2f(0.f, connectionHeight) }),
        xy::Physics::CollisionEdgeShape({ sf::Vector2f(connectionWidth, 0.f), sf::Vector2f(connectionWidth, connectionBend), sf::Vector2f(connectionWidth + connectionGap, connectionHeight) })
    };

    ec.bottom =
    {
        xy::Physics::CollisionEdgeShape({ sf::Vector2f(0.f, sectionSize - connectionHeight), sf::Vector2f(0.f, sectionSize) }),
        xy::Physics::CollisionEdgeShape({ sf::Vector2f(connectionWidth + connectionGap, sectionSize - connectionHeight), sf::Vector2f(connectionWidth, sectionSize - connectionBend), sf::Vector2f(connectionWidth, sectionSize) })
    };
    m_edges.insert(std::make_pair(0x4, ec));
}

//public
void TrackSection::cacheParts(const std::vector<sf::Uint8>& ids)
{
    m_uids = ids;

    //TODO move building edge shapes here


    //TODO build meshes as needed

}

xy::Entity::Ptr TrackSection::create(xy::MessageBus& mb, float height)
{   
    XY_ASSERT(!m_uids.empty(), "parts not yet cached!");
    
    auto body = xy::Component::create<xy::Physics::RigidBody>(mb, xy::Physics::BodyType::Kinematic);
    
    auto uid = m_uids[m_index];
    m_index = (m_index + 1) % m_uids.size();

    //top two points of centre part
    sf::Vector2f tl(sectionSize, connectionHeight);
    sf::Vector2f tr(0.f, connectionHeight);

    //upper half of ID represents top 3 connections
    auto bits = uid & 0xf;
    XY_ASSERT(bits != 0 && bits != 0x8, "can't have empty segments");
    if (bits & 0x4)
    {
        //top left
        /*xy::Physics::CollisionEdgeShape es(connections[0].leftEdge);
        body->addCollisionShape(es);
        es.setPoints(connections[0].rightEdge);
        body->addCollisionShape(es);*/
        for (const auto& ec : m_edges[0x4].top) body->addCollisionShape(ec);

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
    if (bits == (0x4 | 0x1))
    {
        //we have left and right but no middle
        xy::Physics::CollisionEdgeShape es({ sf::Vector2f(connectionWidth + connectionGap, connectionHeight), sf::Vector2f((connectionWidth  * 2.f) + connectionGap, connectionHeight) });
        body->addCollisionShape(es);
    }

    //bottom two points of centre part
    sf::Vector2f bl(sectionSize, connectionHeight * 3.f);
    sf::Vector2f br(0.f, connectionHeight * 3.f);

    //else bottom 3
    bits = (uid & 0xf0) >> 4;
    XY_ASSERT(bits != 0 && bits != 0x8, "can't have empty segments");

    if (bits & 0x4)
    {
        //bottom left
        /*xy::Physics::CollisionEdgeShape es(connections[3].leftEdge);
        body->addCollisionShape(es);
        es.setPoints(connections[3].rightEdge);
        body->addCollisionShape(es);*/
        for (const auto& ec : m_edges[0x4].bottom) body->addCollisionShape(ec);

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
    if (bits == (0x4 | 0x1))
    {
        //we have left and right but no middle
        xy::Physics::CollisionEdgeShape es({ sf::Vector2f(connectionWidth + connectionGap, sectionSize - connectionHeight), sf::Vector2f((connectionWidth  * 2.f) + connectionGap, sectionSize - connectionHeight) });
        body->addCollisionShape(es);
    }

    //create centre part
    xy::Physics::CollisionEdgeShape es({ tl, bl });
    body->addCollisionShape(es);
    es.setPoints({ tr, br });
    body->addCollisionShape(es);
    body->setLinearVelocity({ 0.f, m_initialVelocity });


    auto controller = xy::Component::create<SectionController>(mb, *this);

    auto entity = xy::Entity::create(mb);
    entity->setPosition((xy::DefaultSceneSize.x - sectionSize) / 2.f, height);
    entity->addComponent(body);
    entity->addComponent(controller);

    return std::move(entity);
}

void TrackSection::update(float dt)
{
    m_initialVelocity += dt * speedIncrease;
    REPORT("track velocity", std::to_string(m_initialVelocity));
}

float TrackSection::getSectionSize()
{
    return sectionSize;
}

float TrackSection::getSpeedIncrease()
{
    return speedIncrease;
}