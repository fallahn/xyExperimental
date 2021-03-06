/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

DoodleChum - Zlib license.

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

#include <Game.hpp>
#include <WorldClientState.hpp>
#include <IntroState.hpp>
#include <MenuState.hpp>

#include <xygine/KeyBinds.hpp>
#include <xygine/Console.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Image.hpp>

namespace
{
    float gameSpeed = 1.f;

    const std::string cfgName("default.xfg");
}

Game::Game()
    : //xy::App   (/*sf::ContextSettings(0, 0, 0, 3, 2, sf::ContextSettings::Core)*/),
    m_stateStack({ getRenderWindow(), *this })
{
    xy::Console::addCommand("gametime", [](const std::string& value)
    {
        float speed = 1.f;
        try
        {
            speed = std::max(0.f, std::stof(value.c_str()));
        }
        catch (...)
        {
            xy::Console::print(value + " invalid value.");
        }
        gameSpeed = speed;
    });
}

//private
void Game::handleEvent(const sf::Event& evt)
{    
    m_stateStack.handleEvent(evt);
}

void Game::handleMessage(const xy::Message& msg)
{
    switch (msg.id)
    {
    case xy::Message::Type::UIMessage:
    {
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        case xy::Message::UIEvent::ResizedWindow:
            m_stateStack.updateView();
            break;
        default: break;
        }
        break;
    }
    default: break;
    }
    
    m_stateStack.handleMessage(msg);
}

void Game::updateApp(float dt)
{
#ifdef _DEBUG_
    dt *= gameSpeed;
#endif //_DEBUG_
    m_stateStack.update(dt);
}

void Game::draw()
{
    m_stateStack.draw();
}

void Game::initialise()
{    
    //check config
    if (!m_config.load(cfgName))
    {
        m_config.insert("play_full_tracks", false);
        m_config.insert("use_shadowmapping", true);
        m_config.insert("play_minigame", true);
        m_config.save(cfgName);
    }
    
    //preload the resources for the menu
    m_textureResource.get("assets/images/ui/menu_background.png");
    m_textureResource.get("assets/images/ui/menu_tabs.png");
    m_textureResource.get("assets/images/ui/start_button.png");
    m_textureResource.get("assets/images/ui/small_button.png");
    m_textureResource.get("assets/images/ui/checkbox.png");
    m_textureResource.get("assets/images/ui/scroll_arrow.png");
    m_textureResource.get("assets/images/ui/menu_pic.png");

    registerStates();

#ifdef _DEBUG_
    m_stateStack.pushState(States::WorldClient);
    setWindowTitle("DoodleBob! (debug build)");
#else
    m_stateStack.pushState(States::Intro);
    setWindowTitle("DoodleBob!");
#endif //_DEBUG_
    getRenderWindow().setKeyRepeatEnabled(false);

    if (!xy::Input::load()) xy::Input::save();
    setWindowIcon("assets/images/ui/window.png");
}

void Game::finalise()
{
    m_stateStack.clearStates();
    m_stateStack.applyPendingChanges();

    m_config.save(cfgName);
}

void Game::registerStates()
{
    m_stateStack.registerState<IntroState>(States::ID::Intro);
    m_stateStack.registerState<WorldClientState>(States::ID::WorldClient);
    m_stateStack.registerState<MenuState>(States::ID::Menu, m_fontResource.get("assets/fonts/FallahnHand.ttf"), m_textureResource, m_config);
}
