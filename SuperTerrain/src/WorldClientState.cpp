/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

SuperTerrain - Zlib license.

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
#include <TerrainComponent.hpp>

#include <xygine/App.hpp>
#include <xygine/Command.hpp>
#include <xygine/components/Camera.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Reports.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace
{
    const int playerID = 300;
}

WorldClientState::WorldClientState(xy::StateStack& stateStack, Context context)
    : State     (stateStack, context),
    m_messageBus(context.appInstance.getMessageBus()),
    m_scene     (m_messageBus)
{
    m_scene.setView(context.defaultView);
    
    auto tc = xy::Component::create<TerrainComponent>(m_messageBus);
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(tc);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);


    auto cam = xy::Component::create<xy::Camera>(m_messageBus, context.defaultView);
    auto dwb = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(m_messageBus);
    dwb->getDrawable().setRadius(10.f);
    dwb->getDrawable().setOrigin(10.f, 10.f);
    dwb->getDrawable().setFillColor(sf::Color::Red);

    entity = xy::Entity::create(m_messageBus);
    m_scene.setActiveCamera(entity->addComponent(cam));
    entity->addComponent(dwb);
    entity->addCommandCategories(playerID);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
}

//public
bool WorldClientState::update(float dt)
{
    xy::Command cmd;
    cmd.category = playerID;
    cmd.action = [this](xy::Entity& entity, float dt)
    {
        float speed = 500.f;
        sf::Vector2f velocity;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) velocity.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) velocity.x += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) velocity.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) velocity.y += 1.f;

        if (xy::Util::Vector::lengthSquared(velocity) > 1.f)
        {
            velocity = xy::Util::Vector::normalise(velocity);
        }

        entity.move(velocity * speed * dt);
        REPORT("Position", std::to_string(entity.getWorldPosition().x) + ", " + std::to_string(entity.getWorldPosition().y));
    };
    m_scene.sendCommand(cmd);
    
    m_scene.update(dt);
    return false;
}

bool WorldClientState::handleEvent(const sf::Event& evt)
{
    return false;
}

void WorldClientState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
}

void WorldClientState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
}

//private