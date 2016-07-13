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
#include <TrackMeshBuilder.hpp>

#include <xygine/Assert.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Reports.hpp>
#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>

#include <array>
#include <vector>

#include <TrackConsts.inl>

namespace
{
    const float speedIncrease = 5.f;

    //left to right, top to bottom
    TrackSection::PointData connections;
    bool created = false;
}

TrackSection::TrackSection(xy::MeshRenderer& mr)
    : m_meshRenderer    (mr),
    m_index             (0u), 
    m_trackMaterial     (nullptr),
    m_barrierMaterial   (nullptr),
    m_initialVelocity   (600.f)
{
    if (!created)
    {
        created = true;
        connections[0].first = { sf::Vector2f(), sf::Vector2f(0.f, connectionHeight) };
        connections[0].second = { sf::Vector2f(connectionWidth, 0.f), sf::Vector2f(connectionWidth, connectionBend), sf::Vector2f(connectionWidth + connectionGap, connectionHeight) };

        connections[1].first = { sf::Vector2f(connectionWidth + connectionGap, 0.f), sf::Vector2f(connectionWidth + connectionGap, connectionHeight) };
        connections[1].second = { sf::Vector2f((connectionWidth * 2.f) + connectionGap, 0.f), sf::Vector2f((connectionWidth * 2.f) + connectionGap, connectionHeight) };

        connections[2].first = { sf::Vector2f((connectionWidth * 2.f) + connectionGap, connectionHeight), sf::Vector2f(sectionSize - connectionWidth, connectionBend), sf::Vector2f(sectionSize - connectionWidth, 0.f) };
        connections[2].second = { sf::Vector2f(sectionSize, 0.f), sf::Vector2f(sectionSize, connectionHeight) };

        connections[3].first = { sf::Vector2f(0.f, sectionSize - connectionHeight), sf::Vector2f(0.f, sectionSize) };
        connections[3].second = { sf::Vector2f(connectionWidth + connectionGap, sectionSize - connectionHeight), sf::Vector2f(connectionWidth, sectionSize - connectionBend), sf::Vector2f(connectionWidth, sectionSize) };

        connections[4].first = { sf::Vector2f(connectionWidth + connectionGap, sectionSize - connectionHeight), sf::Vector2f(connectionWidth + connectionGap, sectionSize) };
        connections[4].second = { sf::Vector2f((connectionWidth * 2.f) + connectionGap, sectionSize - connectionHeight), sf::Vector2f((connectionWidth * 2.f) + connectionGap, sectionSize) };

        connections[5].first = { sf::Vector2f((connectionWidth * 2.f) + connectionGap, sectionSize - connectionHeight), sf::Vector2f(sectionSize - connectionWidth, sectionSize - connectionBend), sf::Vector2f(sectionSize - connectionWidth, sectionSize) };
        connections[5].second = { sf::Vector2f(sectionSize, sectionSize - connectionHeight), sf::Vector2f(sectionSize, sectionSize) };
    }
}

//public
void TrackSection::cacheParts(const std::vector<sf::Uint8>& ids)
{
    //build meshes as needed
    std::vector<ModelID> cached;
    for (auto id : ids)
    {
        auto result = std::find_if(cached.begin(), cached.end(), [id](const ModelID& mi) {return mi.id == id; });
        if (result == cached.end())
        {
            //create mesh builder for id
            TrackMeshBuilder tbm(id, connections);

            //cache mesh in mesh renderer and map to id
            m_meshRenderer.loadModel(id, tbm);

            m_uids.emplace_back(id, tbm.getFirstBarrierIndex());
            cached.emplace_back(id, tbm.getFirstBarrierIndex());
        }
        else
        {
            m_uids.emplace_back(result->id, result->barrierOffset);
        }
    }
}

xy::Entity::Ptr TrackSection::create(xy::MessageBus& mb, float height)
{   
    XY_ASSERT(!m_uids.empty(), "parts not yet cached!");
    
    auto body = xy::Component::create<xy::Physics::RigidBody>(mb, xy::Physics::BodyType::Kinematic);
    
    auto uid = m_uids[m_index].id;
    
    //top two points of centre part
    sf::Vector2f tl(sectionSize, connectionHeight);
    sf::Vector2f tr(0.f, connectionHeight);

    //upper half of ID represents top 3 connections
    auto bits = uid & 0xf;
    XY_ASSERT(bits != 0 && bits != 0x8, "can't have empty segments");
    if (bits & 0x4)
    {
        //top left
        xy::Physics::CollisionEdgeShape es(connections[0].first);
        body->addCollisionShape(es);
        es.setPoints(connections[0].second);
        body->addCollisionShape(es);

        if (tl.x > 0.f) tl.x = 0.f;
        if (tr.x < connectionWidth + connectionGap) tr.x = connectionWidth + connectionGap;
    }
    if (bits & 0x2)
    {
        //top middle
        xy::Physics::CollisionEdgeShape es(connections[1].first);
        body->addCollisionShape(es);
        es.setPoints(connections[1].second);
        body->addCollisionShape(es);

        if (tl.x > connectionWidth + connectionGap) tl.x = connectionWidth + connectionGap;
        if (tr.x < (connectionWidth * 2.f) + connectionGap) tr.x = (connectionWidth * 2.f) + connectionGap;
    }
    if (bits & 0x1)
    {
        //top right
        xy::Physics::CollisionEdgeShape es(connections[2].first);
        body->addCollisionShape(es);
        es.setPoints(connections[2].second);
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
        xy::Physics::CollisionEdgeShape es(connections[3].first);
        body->addCollisionShape(es);
        es.setPoints(connections[3].second);
        body->addCollisionShape(es);

        if (bl.x > 0.f) bl.x = 0.f;
        if (br.x < connectionWidth + connectionGap) br.x = connectionWidth + connectionGap;
    }
    if (bits & 0x2)
    {
        //bottom middle
        xy::Physics::CollisionEdgeShape es(connections[4].first);
        body->addCollisionShape(es);
        es.setPoints(connections[4].second);
        body->addCollisionShape(es);

        if (bl.x > connectionWidth + connectionGap) bl.x = connectionWidth + connectionGap;
        if (br.x < (connectionWidth * 2.f) + connectionGap) br.x = (connectionWidth * 2.f) + connectionGap;
    }
    if (bits & 0x1)
    {
        //bottom right
        xy::Physics::CollisionEdgeShape es(connections[5].first);
        body->addCollisionShape(es);
        es.setPoints(connections[5].second);
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
    auto model = m_meshRenderer.createModel(uid, mb);
    if (m_trackMaterial)
    {
        for (auto i = 0u; i < m_uids[m_index].barrierOffset; ++i)
        {
            model->setSubMaterial(*m_trackMaterial, i);
        }
    }
    if (m_barrierMaterial)
    {
        for (auto i = m_uids[m_index].barrierOffset; i < model->getMesh().getSubMeshCount(); ++i)
        {
            model->setSubMaterial(*m_barrierMaterial, i);
        }
    }
    

    auto entity = xy::Entity::create(mb);
    entity->setPosition((xy::DefaultSceneSize.x - sectionSize) / 2.f, height);
    entity->addComponent(body);
    entity->addComponent(controller);
    entity->addComponent(model);

    m_index = (m_index + 1) % m_uids.size(); //remember to do this as late as possible
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