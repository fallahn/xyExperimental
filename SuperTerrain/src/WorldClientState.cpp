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
#include <PlayerController.hpp>

#include <xygine/App.hpp>
#include <xygine/Command.hpp>
#include <xygine/components/Camera.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Reports.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

namespace
{
    const int playerID = 300;

    //xy::Camera* playerCamera = nullptr;
}

WorldClientState::WorldClientState(xy::StateStack& stateStack, Context context)
    : State     (stateStack, context),
    m_messageBus(context.appInstance.getMessageBus()),
    m_scene     (m_messageBus)
{
    m_scene.setView(context.defaultView);
    
    auto tc = xy::Component::create<TerrainComponent>(m_messageBus, context.appInstance);
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(tc);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);



    //auto cam = xy::Component::create<xy::Camera>(m_messageBus, context.defaultView);
    auto dwb = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(m_messageBus);
    dwb->getDrawable().setRadius(20.f);
    dwb->getDrawable().setOrigin(20.f, 20.f);
    dwb->getDrawable().setFillColor(sf::Color::Red);
    dwb->getDrawable().setPointCount(3);
    dwb->getDrawable().setRotation(-30.f);

    auto playerController = xy::Component::create<st::PlayerController>(m_messageBus);

    entity = xy::Entity::create(m_messageBus);
    //playerCamera = entity->addComponent(cam);
    //m_scene.setActiveCamera(playerCamera);
    entity->addComponent(dwb);
    entity->addComponent(playerController);
    entity->addCommandCategories(playerID);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
}

//public
bool WorldClientState::update(float dt)
{
    xy::Command cmd;
    cmd.category = playerID;
    cmd.action = [this](xy::Entity& entity, float dt)
    {
        sf::Uint32 input = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) input |= st::PlayerController::Left;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) input |= st::PlayerController::Right;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) input |= st::PlayerController::Forward;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) input |= st::PlayerController::Back;

        auto direction = xy::App::getMouseWorldPosition() - entity.getWorldPosition();
        auto angle = xy::Util::Vector::rotation(direction);
        input |= (static_cast<sf::Int16>(angle) << 16);
        //REPORT("Input angle", std::to_string(angle));
        //REPORT("Direction", std::to_string(direction.x) + ", " + std::to_string(direction.y));

        entity.getComponent<st::PlayerController>()->setInput(input);
        //REPORT("Position", std::to_string(entity.getWorldPosition().x) + ", " + std::to_string(entity.getWorldPosition().y));
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

    if (msg.id == xy::Message::UIMessage)
    {
        const auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::ResizedWindow:
        {
            /*auto v = playerCamera->getView();
            v.setViewport(getContext().defaultView.getViewport());
            playerCamera->setView(v);*/
            m_scene.setView(getContext().defaultView);
        }
        break;
        }
    }
}

void WorldClientState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    rw.setView(getContext().defaultView);
}

//private