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

#include <RacingState.hpp>

#include <xygine/App.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/mesh/CubeBuilder.hpp>
#include <xygine/components/Model.hpp>

#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>

#include <SFML/Window/Event.hpp>

RacingState::RacingState(xy::StateStack& stateStack, Context context)
    : xy::State     (stateStack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus),
    m_physWorld     (m_messageBus),
    m_meshRenderer  ({ context.appInstance.getVideoSettings().VideoMode.width, context.appInstance.getVideoSettings().VideoMode.height }, m_scene),
    m_trackSection  (m_meshRenderer)
{
    launchLoadingScreen();

    m_scene.setView(context.defaultView);
    m_scene.getSkyLight().setIntensity(0.76f);
    m_scene.getSkyLight().setDiffuseColour({ 255, 255, 240 });
    m_scene.getSkyLight().setSpecularColour({ 220, 255, 251 });
    m_scene.getSkyLight().setDirection({ 0.2f, 0.4f, -0.7f });

    init();
    addBodies();

    quitLoadingScreen();
}

//public
bool RacingState::handleEvent(const sf::Event& evt)
{   
    return true;
}

void RacingState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
    m_meshRenderer.handleMessage(msg);
}

bool RacingState::update(float dt)
{
    m_trackSection.update(dt);
    m_scene.update(dt);
    m_meshRenderer.update();
    return true;
}

void RacingState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);

    rw.setView(getContext().defaultView);
    rw.draw(m_meshRenderer);

    /*rw.setView(m_scene.getView());
    rw.draw(m_physWorld);*/
}

//private
void RacingState::init()
{
    m_physWorld.setGravity({ 0.f, 0.f });
    m_physWorld.setPixelScale(30.f);

    //generate a sequence of IDs using the
    //exist value of the last section as the entrance of the new one.
    std::vector<sf::Uint8> sequenceIDs;
    sequenceIDs.push_back(0x77);

    sf::Uint8 lastExit = 0x7;
    for (auto i = 0; i < 50; ++i) //TODO set up a variable somewhere for the number of track pieces
    {
        sf::Uint8 uid = (lastExit << 4) | xy::Util::Random::value(1, 7);
        lastExit = uid & 0xf;
        sequenceIDs.push_back(uid);
    }
    sequenceIDs.push_back((lastExit << 4) | 0x7);
    m_trackSection.cacheParts(sequenceIDs);

    //set up the materials for the track parts
    m_shaderResource.preload(ShaderID::ColouredSmooth, DEFERRED_COLOURED_VERTEX, DEFERRED_COLOURED_FRAGMENT);
    m_shaderResource.preload(ShaderID::TexturedBumped, DEFERRED_TEXTURED_BUMPED_VERTEX, DEFERRED_TEXTURED_BUMPED_FRAGMENT);
    m_shaderResource.preload(ShaderID::ShadowCaster, SHADOW_VERTEX, xy::Shader::Mesh::ShadowFragment);

    auto& barrierMaterial = m_materialResource.add(MaterialID::Barrier, m_shaderResource.get(ShaderID::TexturedBumped));
    barrierMaterial.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    barrierMaterial.addProperty({ "u_diffuseMap", m_textureResource.get("assets/metal_diffuse.png") });
    barrierMaterial.addProperty({ "u_normalMap", m_textureResource.get("assets/metal_normal.png") });
    barrierMaterial.addProperty({ "u_maskMap", m_textureResource.get("assets/metal_mask.png") });
    //barrierMaterial.addProperty({ "u_colour", sf::Color(170, 20, 20) });
    barrierMaterial.addRenderPass(xy::RenderPass::ID::ShadowMap, m_shaderResource.get(ShaderID::ShadowCaster));
    barrierMaterial.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);
    //TODO disable face culling per pass
    m_trackSection.setBarrierMaterial(barrierMaterial);

    auto& trackMaterial = m_materialResource.add(MaterialID::Track, m_shaderResource.get(ShaderID::TexturedBumped));
    trackMaterial.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    trackMaterial.addProperty({ "u_diffuseMap", m_textureResource.get("assets/metal_diffuse.png") });
    trackMaterial.addProperty({ "u_normalMap", m_textureResource.get("assets/metal_normal.png") });
    trackMaterial.addProperty({ "u_maskMap", m_textureResource.get("assets/metal_mask.png") });
    m_trackSection.setTrackMaterial(trackMaterial);

    auto trackSection = m_trackSection.create(m_messageBus);
    m_scene.addEntity(trackSection, xy::Scene::Layer::FrontRear);

    trackSection = m_trackSection.create(m_messageBus, -TrackSection::getSectionSize());
    m_scene.addEntity(trackSection, xy::Scene::Layer::FrontRear);

    xy::CubeBuilder cb(28.f);
    m_meshRenderer.loadModel(ModelID::Cube, cb);

    auto& cubeMaterial = m_materialResource.add(MaterialID::Thing, m_shaderResource.get(ShaderID::ColouredSmooth));
    cubeMaterial.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    cubeMaterial.addRenderPass(xy::RenderPass::ID::ShadowMap, m_shaderResource.get(ShaderID::ShadowCaster));
    cubeMaterial.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);
}

void RacingState::addBodies()
{
    std::function<void()> addBody = [this]()
    {
        auto smallBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
        smallBody->fixedRotation(true);

        //xy::Physics::CollisionCircleShape cs(14.f);
        xy::Physics::CollisionRectangleShape cs({ 28.f, 28.f }, { -14.f, -14.f });
        cs.setDensity(0.4f);
        cs.setRestitution(0.6f);
        
        xy::Physics::CollisionFilter cf;
        cf.categoryFlags |= PhysCat::SmallBody;
        cf.maskFlags = 0xffff;// PhysCat::Trap | PhysCat::Wall;
        cs.setFilter(cf);

        smallBody->addCollisionShape(cs);

        auto model = m_meshRenderer.createModel(ModelID::Cube, m_messageBus);
        model->setBaseMaterial(m_materialResource.get(MaterialID::Thing));
        model->setPosition({ 0.f, 0.f, 14.f });

        auto entity = xy::Entity::create(m_messageBus);
        entity->setPosition(xy::Util::Random::value(200.f, 1600.f), 1020.f);
        auto b = entity->addComponent(smallBody);
        entity->addComponent(model);
        m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
        b->applyForceToCentre({ 0.f, xy::Util::Random::value(1500.f, 2500.f) });
    };
    for (auto i = 0u; i < 20u; ++i) addBody();

    //deals with the ball bounds
    auto anchorBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Static);
    xy::Physics::CollisionRectangleShape cr({ xy::DefaultSceneSize.x, 10.f }, { 0.f, 1120.f });
    xy::Physics::CollisionFilter cf;
    cf.categoryFlags |= PhysCat::Trap;
    cf.maskFlags |= PhysCat::SmallBody;
    cr.setFilter(cf);
    anchorBody->addCollisionShape(cr);

    cr.setRect({ xy::DefaultSceneSize.x, 60.f }, { 0.f, -80.f });
    //cr.setIsSensor(true);
    //cf.categoryFlags = PhysCat::Destroyer;
    cr.setFilter(cf);
    anchorBody->addCollisionShape(cr);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(anchorBody);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);


    //handle balls leaving the area by deleting
    //std::function<void(xy::Physics::Contact&)> cb =
    //    [this](xy::Physics::Contact& contact)
    //{
    //    if (contact.getCollisionShapeA()->getFilter().categoryFlags & PhysCat::Destroyer)
    //    {
    //        xy::Command cmd;
    //        cmd.entityID = contact.getCollisionShapeB()->getRigidBody()->getParentUID();
    //        cmd.action = [](xy::Entity& ent, float) { ent.destroy(); };
    //        m_scene.sendCommand(cmd);
    //        //LOG("Destroyed ball!", xy::Logger::Type::Info);
    //    }
    //    else if (contact.getCollisionShapeB()->getFilter().categoryFlags & PhysCat::Destroyer)
    //    {
    //        xy::Command cmd;
    //        cmd.entityID = contact.getCollisionShapeA()->getRigidBody()->getParentUID();
    //        cmd.action = [](xy::Entity& ent, float) { ent.destroy(); };
    //        m_scene.sendCommand(cmd);
    //        contact.getCollisionShapeA()->getRigidBody()->destroy();
    //        //LOG("Destroyed ball!", xy::Logger::Type::Info);
    //    }
    //};
    //m_physWorld.addContactBeginCallback(cb);
}