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

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

Sandbox::Sandbox(xy::MessageBus& mb, UserInterface& ui)
    : m_messageBus  (mb),
    m_ui            (ui),
    m_scene         (mb)
{

}

Sandbox::~Sandbox()
{
    m_ui.removeItem(this);
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