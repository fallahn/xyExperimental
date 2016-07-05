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

#include <xygine/MessageBus.hpp>
#include <xygine/Entity.hpp>
#include <xygine/imgui/imgui.h>
#include <xygine/Command.hpp>
#include <xygine/components/Camera.hpp>
#include <xygine/App.hpp>
#include <xygine/Reports.hpp>
#include <xygine/util/Vector.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace
{
    //Parameters parameters;
    //TrackRenderer* renderer = nullptr;

    const std::string trackDir("tracks/");
    std::vector<std::string> trackFiles;

    std::array<sf::Vertex, 4u> backgroundQuad =
    {
        sf::Vertex({0.f, 0.f}, sf::Color(0, 120, 255, 120)),
        sf::Vertex({ xy::DefaultSceneSize.x, 0.f }, sf::Color(0, 120, 255, 120)),
        sf::Vertex(xy::DefaultSceneSize, sf::Color(0, 120, 255, 120)),
        sf::Vertex({ 0.f, xy::DefaultSceneSize.y }, sf::Color(0, 120, 255, 120))
    };

    xy::Camera* camera = nullptr;

    enum CommandID
    {
        Camera = 0x1,
    };
    const float defaultZoom = 0.5f;
}

Sandbox::Sandbox(xy::MessageBus& mb, UserInterface& ui, sf::RenderWindow& rw)
    : m_messageBus  (mb),
    m_ui            (ui),
    m_renderWindow  (rw),
    m_scene         (mb),
    m_physWorld     (mb)
{
    //parameters.load("default.tgn");

    //if (!xy::FileSystem::directoryExists(trackDir))
    //{
    //    xy::FileSystem::createDirectory(trackDir);
    //}
    //updateFileList();

    //m_ui.addItem([this]()
    //{
    //    nim::InputInt("Min Points", &parameters.minPoints, 1, 10);
    //    nim::InputInt("Max Points", &parameters.maxPoints, 1, 10);
    //    nim::InputFloat("Min Seg Length", &parameters.minSegmentLength, 1.f, 10.f);
    //    nim::InputFloat("Max Seg Length", &parameters.maxSegmentLength, 1.f, 10.f);
    //    nim::InputFloat("Curviness", &parameters.curviness, 0.1f, 1.f);
    //    nim::InputFloat("Max Angle", &parameters.maxAngle, 1.f, 10.f);
    //    nim::Checkbox("No Crossings", &parameters.noCrossing);

    //    //save / load params
    //    if (nim::Button("Save", {70.f, 20.f}))
    //    {
    //        nim::OpenPopup("Save File");
    //    }
    //    if (nim::BeginPopupModal("Save File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    //    {
    //        static char filename[50];

    //        nim::InputText("File Name", filename, 49);
    //        if (nim::Button("OK"))
    //        {
    //            parameters.save(trackDir + std::string(filename) + ".tgn");
    //            updateFileList();
    //            nim::CloseCurrentPopup();
    //        }
    //        nim::SameLine();
    //        if (nim::Button("Cancel"))
    //        {
    //            nim::CloseCurrentPopup();
    //        }
    //        nim::EndPopup();
    //    }

    //    nim::SameLine();
    //    if (nim::Button("Load", { 70.f, 20.f }))
    //    {
    //        nim::OpenPopup("Load File");
    //    }
    //    if (nim::BeginPopupModal("Load File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    //    {
    //        static int index;
    //        nim::Combo("Select File", &index, [](void* data, int idx, const char** out_text)
    //        {
    //            *out_text = ((const std::vector<std::string>*)data)->at(idx).c_str();
    //            return true;
    //        }, (void*)&trackFiles, trackFiles.size());

    //        if (nim::Button("OK"))
    //        {
    //            parameters.load(trackDir + trackFiles[index]);
    //            nim::CloseCurrentPopup();
    //        }
    //        nim::SameLine();
    //        if (nim::Button("Cancel"))
    //        {
    //            nim::CloseCurrentPopup();
    //        }
    //        nim::EndPopup();
    //    }

    //    //generate / export results
    //    if (nim::Button("Generate", { 70.f, 20.f }))
    //    {
    //        m_trackGenerator.generate(parameters);
    //        renderer->setData(m_trackGenerator.getData());
    //    }
    //    nim::SameLine();
    //    if (nim::Button("Export", { 70.f, 20.f }))
    //    {
    //        nim::OpenPopup("Export File");
    //    }
    //    if (nim::BeginPopupModal("Export File"))
    //    {
    //        static char filename[50];

    //        nim::InputText("File Name", filename, 49);
    //        if (nim::Button("OK"))
    //        {
    //            //TODO export the generated data file
    //            nim::CloseCurrentPopup();
    //        }
    //        nim::SameLine();
    //        if (nim::Button("Cancel"))
    //        {
    //            nim::CloseCurrentPopup();
    //        }
    //        nim::EndPopup();
    //    }
    //}, this);

    initScene();
}

Sandbox::~Sandbox()
{
    //parameters.save("default.tgn");
    m_ui.removeItems(this);
}

//public
void Sandbox::update(float dt)
{
    m_scene.update(dt);
}

void Sandbox::handleEvent(const sf::Event& evt)
{
    static sf::Vector2f mousePos;
    static float zoom = defaultZoom;
    REPORT("zoom", std::to_string(zoom));

    if (evt.type == sf::Event::MouseWheelMoved)
    {
        m_renderWindow.setView(camera->getView());
        auto startPos = xy::App::getMouseWorldPosition();
        
        if (evt.mouseWheel.delta > 0)
        {
            zoom *= 1.1f;
        }
        else
        {
            zoom *= 0.9f;
        }
        camera->setZoom(zoom);
        m_renderWindow.setView(camera->getView());
        auto movement = startPos - xy::App::getMouseWorldPosition();

        xy::Command cmd;
        cmd.category = CommandID::Camera;
        cmd.action = [movement](xy::Entity& entity, float)
        {
            entity.move(movement);
        };
        m_scene.sendCommand(cmd);
    }
    else if (evt.type == sf::Event::MouseMoved)
    {       
        if (sf::Mouse::isButtonPressed(sf::Mouse::Middle))
        {
            sf::Vector2f evtPos = { static_cast<float>(evt.mouseMove.x), static_cast<float>(evt.mouseMove.y) };
            auto movement = mousePos - evtPos;

            //prevent random movement spikes
            if (xy::Util::Vector::lengthSquared(movement) > 10000.f) return;

            movement *= (1.f / zoom);

            xy::Command cmd;
            cmd.category = CommandID::Camera;
            cmd.action = [movement](xy::Entity& entity, float)
            {
                entity.move(movement);
                REPORT("Camera Position", std::to_string(entity.getPosition().x) + ", " + std::to_string(entity.getPosition().y));
            };
            m_scene.sendCommand(cmd);

            mousePos = evtPos;

            REPORT("Camera Movement", std::to_string(movement.x) + ", " + std::to_string(movement.y));
        }
    }
    else if (evt.type == sf::Event::MouseButtonPressed)
    {
        if (evt.mouseButton.button == sf::Mouse::Middle)
        {
            mousePos = { static_cast<float>(evt.mouseButton.x), static_cast<float>(evt.mouseButton.y) };
        }
    }
}

void Sandbox::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
}

//private
void Sandbox::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.setView(camera->getView());
    rt.draw(backgroundQuad.data(), backgroundQuad.size(), sf::Quads);
    rt.draw(m_scene);
    rt.setView(camera->getView());
    rt.draw(m_physWorld);
}

void Sandbox::updateFileList()
{
    trackFiles = xy::FileSystem::listFiles(trackDir);
    trackFiles.erase(std::remove_if(trackFiles.begin(), trackFiles.end(),
        [](const std::string& str)
    {
        return xy::FileSystem::getFileExtension(str) != ".tgn";
    }), trackFiles.end());
}

void Sandbox::initScene()
{
    auto cam = xy::Camera::create<xy::Camera>(m_messageBus, m_renderWindow.getView());
    cam->setZoom(defaultZoom);
    auto entity = xy::Entity::create(m_messageBus);
    camera = entity->addComponent(cam);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    entity->addCommandCategories(CommandID::Camera);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);
    m_scene.setActiveCamera(camera);

    m_physWorld.setGravity({ 0.f, 0.f });
    m_physWorld.setPixelScale(30.f);

    auto trackSection = m_trackSection.create(0x17, m_messageBus);
    m_scene.addEntity(trackSection, xy::Scene::Layer::FrontRear);

    trackSection = m_trackSection.create(0x61, m_messageBus, -1024.f);
}