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

#ifndef PS_SANDBOX_HPP_
#define PS_SANDBOX_HPP_

#include <xygine/Scene.hpp>
#include <xygine/physics/World.hpp>

#include <SFML/Graphics/Drawable.hpp>

namespace xy
{
    class Message;
    class MessageBus;
}

namespace sf
{
    class Event;
}

class UserInterface;
class Sandbox final : public sf::Drawable
{
public:
    Sandbox(xy::MessageBus&, UserInterface&);
    ~Sandbox();

    void update(float);
    void handleEvent(const sf::Event&);
    void handleMessage(const xy::Message&);

    void setView(const sf::View&);

private:

    xy::MessageBus& m_messageBus;
    UserInterface& m_ui;
    xy::Scene m_scene;
    xy::Physics::World m_physWorld;
    sf::View m_view;

    void draw(sf::RenderTarget&, sf::RenderStates) const override;
    void setupVehicles();
    void updateFileList();
};

#endif //PS_SANDBOX_HPP_