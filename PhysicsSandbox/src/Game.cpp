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

#include <Game.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    sf::View updateView(sf::Vector2u size)
    {
        float ratio = static_cast<float>(size.y) / size.x;
        const float width = 1280.f;
        const float height = width * ratio;

        return sf::View({ 0.f, 0.f, width, height });
    }
}

Game::Game()
    : m_userInterface   (*this),
    m_sandbox           (getMessageBus(), m_userInterface)
{
    setMouseCursorVisible(true);

    auto& window = getRenderWindow();
    m_sandbox.setView(updateView(window.getSize()));
}

//private
void Game::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::Escape:
            xy::App::quit();
            break;
        }
    }

    m_sandbox.handleEvent(evt);
}

void Game::handleMessage(const xy::Message& msg)
{
    if (msg.id == xy::Message::UIMessage)
    {
        const auto& msgData = msg.getData<xy::Message::UIEvent>();
        if (msgData.type == xy::Message::UIEvent::ResizedWindow)
        {
            auto& window = getRenderWindow();
            m_sandbox.setView(updateView(window.getSize()));
        }
    }
    m_sandbox.handleMessage(msg);
}

void Game::updateApp(float dt)
{
    m_sandbox.update(dt);
}

void Game::draw()
{
    getRenderWindow().draw(m_sandbox);
}