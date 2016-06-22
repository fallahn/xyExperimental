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

#include <UserInterface.hpp>

#include <xygine/App.hpp>
#include <xygine/Config.hpp>
#include <xygine/imgui/imgui.h>

namespace
{
    struct Item final
    {
        std::function<void()> draw;
        const void* owner = nullptr;
    };

    std::vector<Item> items;
    bool showVideoOptions = false;
    bool fullScreen = false;

    std::vector<sf::VideoMode> modes;
    int currentResolution = 0;
    char resolutionNames[300];
}

UserInterface::UserInterface(xy::App& app)
    : m_app (app)
{     
    //main window
    xy::App::addUserWindow([this]()
    {
        //nim::ShowTestWindow();
        nim::SetNextWindowSize({ 300.f, 400.f });
        if (!nim::Begin("Menu", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_ShowBorders))
        {
            //window is collapsed so skip out
            nim::End();
            return;
        }
        if (nim::BeginMenuBar())
        {
            if (nim::BeginMenu("Options"))
            {
                if (nim::MenuItem("Video", nullptr, &showVideoOptions))
                {
                    //select active mode
                    const auto& activeMode = m_app.getVideoSettings().VideoMode;
                    for (auto i = 0u; i < modes.size(); ++i)
                    {
                        if (modes[i] == activeMode)
                        {
                            currentResolution = i;
                            break;
                        }
                    }
                }
                nim::EndMenu();
            }

            if (nim::BeginMenu("Quit"))
            {
                xy::App::quit();
                nim::EndMenu();
            }
            nim::EndMenuBar();
        }

        nim::Separator();
        //draw added items in sub window
        nim::BeginChild("custom");
        for (const auto& item : items) item.draw();
        nim::EndChild();

        nim::End();
    });

    //video options
    modes = m_app.getVideoSettings().AvailableVideoModes;
    int i = 0;
    for (const auto& mode : modes)
    {
        if (mode.bitsPerPixel == 32u && mode.isValid())
        {
            std::string width = std::to_string(mode.width);
            std::string height = std::to_string(mode.height);

            for (char c : width)
            {
                resolutionNames[i++] = c;
            }
            resolutionNames[i++] = ' ';
            resolutionNames[i++] = 'x';
            resolutionNames[i++] = ' ';
            for (char c : height)
            {
                resolutionNames[i++] = c;
            }
            resolutionNames[i++] = '\0';
        }
    }

    xy::App::addUserWindow([this]()
    {
        if (!showVideoOptions) return;

        nim::SetNextWindowSize({ 300.f, 100.f });
        nim::Begin("Video Options", &showVideoOptions, ImGuiWindowFlags_ShowBorders);

        nim::Combo("Resolution", &currentResolution, resolutionNames);

        nim::Checkbox("Full Screen", &fullScreen);
        if (nim::Button("Apply", { 50.f, 20.f }))
        {
            //apply settings
            xy::App::VideoSettings settings;
            settings.WindowStyle = (fullScreen) ? sf::Style::Fullscreen : sf::Style::Close;
            settings.VideoMode = modes[currentResolution];
            m_app.applyVideoSettings(settings);
        }
        nim::End();
    });
}

//public
void UserInterface::addItem(const std::function<void()>& item, const void* owner)
{
    items.emplace_back();
    items.back().draw = item;
    items.back().owner = owner;
}

void UserInterface::removeItem(const void* owner)
{
    XY_ASSERT(owner, "don't do this!");

    items.erase(std::remove_if(items.begin(), items.end(), 
        [owner](const Item& item)
    {
        return item.owner == owner;
    }), items.end());
}