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

#include <Sandbox.hpp>
#include <UserInterface.hpp>
#include <VehicleControllerB2D.hpp>

#include <xygine/MessageBus.hpp>
#include <xygine/Entity.hpp>
#include <xygine/FileSystem.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>
#include <xygine/physics/CollisionPolygonShape.hpp>
#include <xygine/physics/CollisionEdgeShape.hpp>

#include <xygine/imgui/imgui.h>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    VehicleControllerB2D* car = nullptr;
    VehicleControllerB2D* bike = nullptr;
    VehicleControllerB2D* ship = nullptr;
    
    VehicleControllerB2D* controller = nullptr;
    sf::Uint8 input = 0;

    VehicleControllerB2D::Parameters vehicleParameters;

    std::vector<std::string> vehicleFiles;
    const std::string vehicleDir("vehicledefs/");
}

Sandbox::Sandbox(xy::MessageBus& mb, UserInterface& ui)
    : m_messageBus  (mb),
    m_ui            (ui),
    m_scene         (mb),
    m_physWorld     (mb)
{
    updateFileList();
    
    setupVehicles();

    ui.addItem([this]()
    {
        nim::InputFloat("Max Forward", &vehicleParameters.maxForwardSpeed, 10.f, 50.f);
        nim::InputFloat("Max Backward", &vehicleParameters.maxBackwardSpeed, 10.f, 50.f);
        nim::InputFloat("Drive Force", &vehicleParameters.driveForce, 10.f, 50.f);
        nim::InputFloat("Turn Speed", &vehicleParameters.turnSpeed, 0.01f, 0.05f);
        nim::InputFloat("Drag", &vehicleParameters.drag, 0.1f, 1.f);
        nim::InputFloat("Body Density", &vehicleParameters.density, 0.1f, 1.f);
        nim::InputFloat("Angular Friction", &vehicleParameters.angularFriction, 0.1f, 1.f);
        nim::InputFloat("Grip", &vehicleParameters.grip, 0.1f, 1.f);

        nim::Combo("Vehicle", (int*)&vehicleParameters.type, "Bike\0Car\0Ship");
        switch (vehicleParameters.type)
        {
        default: case VehicleControllerB2D::Type::Bike:
            controller = bike;
            break;
        case VehicleControllerB2D::Type::Car:
            controller = car;
            break;
        case VehicleControllerB2D::Type::Ship:
            controller = ship;
            break;
        }

        nim::Separator();
        if (nim::Button("Save"))
        {
            nim::OpenPopup("Save File");
        }
        if (nim::BeginPopupModal("Save File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char filename[50];
            
            nim::InputText("File Name", filename, 49);
            if (nim::Button("OK"))
            {
                vehicleParameters.save(vehicleDir + std::string(filename) + ".car");
                updateFileList();
                nim::CloseCurrentPopup();
            }
            nim::SameLine();
            if (nim::Button("Cancel"))
            {
                nim::CloseCurrentPopup();
            }
            nim::EndPopup();
        }
        nim::SameLine();
        if (nim::Button("Load"))
        {
            nim::OpenPopup("Load File");
        }
        if (nim::BeginPopupModal("Load File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static int index;
            nim::Combo("Select File", &index, [](void* data, int idx, const char** out_text) 
            { 
                *out_text = ((const std::vector<std::string>*)data)->at(idx).c_str();
                return true;
            }, (void*)&vehicleFiles, vehicleFiles.size());
            
            if (nim::Button("OK"))
            {
                vehicleParameters.load(vehicleDir + vehicleFiles[index]);
                nim::CloseCurrentPopup();
            }
            nim::SameLine();
            if (nim::Button("Cancel"))
            {
                nim::CloseCurrentPopup();
            }
            nim::EndPopup();
        }

    }, this);

    vehicleParameters.load("default.car");
}

Sandbox::~Sandbox()
{
    vehicleParameters.save("default.car");
    m_ui.removeItems(this);
}

//public
void Sandbox::update(float dt)
{
    controller->setInput(input);
    controller->setParameters(vehicleParameters);
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
            input |= Control::Handbrake;
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
            input &= ~Control::Handbrake;
            break;
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

void Sandbox::setupVehicles()
{
    m_physWorld.setGravity({ 0.f, 0.f });
    
    //bounds
    auto body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Static);
    auto shape = xy::Physics::CollisionRectangleShape({ 10.f, 720.f });
    body->addCollisionShape(shape);
    shape.setRect({ 10.f, 720.f }, { 1270.f, 0.f });
    body->addCollisionShape(shape);
    shape.setRect({ 1260.f, 10.f }, { 10.f, 0.f });
    body->addCollisionShape(shape);
    shape.setRect({ 1260.f, 10.f }, { 10.f, 710.f });
    body->addCollisionShape(shape);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(body);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    //bike
    body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);

    std::vector<sf::Vector2f> points = 
    {
        sf::Vector2f(6.f, 19.f),
        {34.f, 19.f},
        {37.f, 124.f},
        {20.f, 129.f},
        {3.f, 124.f}
    };
    xy::Physics::CollisionPolygonShape ps(points);
    ps.setDensity(1.f);
    body->addCollisionShape(ps);

    shape.setRect({ 28.f, 22.f }, { 6.f, 20.f });
    shape.setDensity(1.5f);
    body->addCollisionShape(shape);

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(512.f, 288.f);
    auto bodyPtr = entity->addComponent(body);

    auto vehicleController = xy::Component::create<VehicleControllerB2D>(m_messageBus, bodyPtr);
    bike = entity->addComponent(vehicleController);
    controller = bike;

    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    //car
    points = 
    {
        {32.f, 4.f},
        {45.f, 4.f},
        {73.f, 31.f},
        {71.f, 128.f},
        {38.f, 172.f},
        {6.f, 128.f},
        {4.f, 31.f}
    };
    ps.setPoints(points);
    body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
    body->addCollisionShape(ps);
    shape.setRect({ 32.f, 40.f }, { 22.5f, 10.f });
    body->addCollisionShape(shape);
    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(780.f, 288.f);
    bodyPtr = entity->addComponent(body);
    vehicleController = xy::Component::create<VehicleControllerB2D>(m_messageBus, bodyPtr);
    car = entity->addComponent(vehicleController);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    //ship
    points = 
    {
        {8.f, 17.f},
        {49.f, 0.f },
        {71.f, 0.f},
        {112.f, 17.f},
        {120.f, 25.f},
        {120.f, 50.f},
        {102.f, 84.f},
        {72.f, 132.f},
        {48.f, 132.f},
        {18.f, 84.f},
        {0.f, 50.f},
        {0.f, 25.f}
    };
    xy::Physics::CollisionEdgeShape es(points, xy::Physics::CollisionEdgeShape::Option::Loop);
    body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
    body->addCollisionShape(es);
    shape.setRect({ 60.f, 80.f }, { 30.f, 20.f });
    body->addCollisionShape(shape);
    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(960.f, 288.f);
    bodyPtr = entity->addComponent(body);
    vehicleController = xy::Component::create<VehicleControllerB2D>(m_messageBus, bodyPtr);
    ship = entity->addComponent(vehicleController);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
}

void Sandbox::updateFileList()
{
    vehicleFiles = xy::FileSystem::listFiles(vehicleDir);
    vehicleFiles.erase(std::remove_if(vehicleFiles.begin(), vehicleFiles.end(),
        [](const std::string& str)
    {
        return xy::FileSystem::getFileExtension(str) != ".car";
    }), vehicleFiles.end());
}