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

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <fstream>

namespace
{
    struct Parameters final
    {
        int minPoints = 40;
        int maxPoints = 80;
        float minSegmentLength = 2.f;
        float maxSegmentLength = 16.f;
        float curviness = 0.3f;
        float maxAngle = 150.f;

        void save(const std::string& path)
        {
            std::ofstream file(path, std::ios::binary);
            if (file.good() && file.is_open())
            {
                file.write((char*)this, sizeof(Parameters));
            }
            else
            {
                xy::Logger::log("Failed saving track gen parameters " + path, xy::Logger::Type::Error);
            }
            file.close();
        }

        void load(const std::string& path)
        {
            std::ifstream file(path, std::ios::binary);
            if (file.good() && file.is_open())
            {
                file.read((char*)this, sizeof(Parameters));
            }
            else
            {
                xy::Logger::log("Failed reading track gen parameters " + path, xy::Logger::Type::Error);
            }
            file.close();
        }
    } parameters;

    const std::string trackDir("tracks/");
    std::vector<std::string> trackFiles;
}

Sandbox::Sandbox(xy::MessageBus& mb, UserInterface& ui)
    : m_messageBus  (mb),
    m_ui            (ui),
    m_scene         (mb)
{
    parameters.load("default.tgn");

    if (!xy::FileSystem::directoryExists(trackDir))
    {
        xy::FileSystem::createDirectory(trackDir);
    }
    updateFileList();

    m_ui.addItem([this]()
    {
        nim::InputInt("Min Points", &parameters.minPoints, 1, 10);
        nim::InputInt("Max Points", &parameters.maxPoints, 1, 10);
        nim::InputFloat("Min Seg Length", &parameters.minSegmentLength, 1.f, 10.f);
        nim::InputFloat("Max Seg Length", &parameters.maxSegmentLength, 1.f, 10.f);
        nim::InputFloat("Curviness", &parameters.curviness, 0.1f, 1.f);
        nim::InputFloat("Max Angle", &parameters.maxAngle, 1.f, 10.f);

        //save / load params
        if (nim::Button("Save", {70.f, 20.f}))
        {
            nim::OpenPopup("Save File");
        }
        if (nim::BeginPopupModal("Save File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char filename[50];

            nim::InputText("File Name", filename, 49);
            if (nim::Button("OK"))
            {
                parameters.save(trackDir + std::string(filename) + ".tgn");
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
        if (nim::Button("Load", { 70.f, 20.f }))
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
            }, (void*)&trackFiles, trackFiles.size());

            if (nim::Button("OK"))
            {
                parameters.load(trackDir + trackFiles[index]);
                nim::CloseCurrentPopup();
            }
            nim::SameLine();
            if (nim::Button("Cancel"))
            {
                nim::CloseCurrentPopup();
            }
            nim::EndPopup();
        }

        //generate / export results
        if (nim::Button("Generate", { 70.f, 20.f }))
        {
            //TODO update track output
        }
        nim::SameLine();
        if (nim::Button("Export", { 70.f, 20.f }))
        {
            nim::OpenPopup("Export File");
        }
        if (nim::BeginPopupModal("Export File"))
        {
            static char filename[50];

            nim::InputText("File Name", filename, 49);
            if (nim::Button("OK"))
            {
                //TODO export the generated data file
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
}

Sandbox::~Sandbox()
{
    parameters.save("default.tgn");
    m_ui.removeItems(this);
}

//public
void Sandbox::update(float dt)
{
    m_scene.update(dt);
}

void Sandbox::handleEvent(const sf::Event& evt)
{
    
}

void Sandbox::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
}

//private
void Sandbox::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_scene);
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