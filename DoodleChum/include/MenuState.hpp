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

#ifndef DC_MENU_STATE_HPP_
#define DC_MENU_STATE_HPP_

#include <StateIDs.hpp>

#include <xygine/State.hpp>
#include <xygine/Resource.hpp>
#include <xygine/ui/Container.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>

class MenuState final : public xy::State
{
public:
    MenuState(xy::StateStack&, Context, const sf::Font&, xy::TextureResource&);
    ~MenuState() = default;

    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    bool update(float) override;
    void draw() override;

    xy::StateID stateID() const override { return States::ID::Menu; }

private:
    xy::MessageBus& m_messageBus;
    xy::TextureResource& m_textureResource;
    const sf::Font& m_font;

    xy::UI::Container m_helpContainer;
    xy::UI::Container m_optionContainer;
    xy::UI::Container m_creditsContainer;
    xy::UI::Container* m_currentContainer;
    
    sf::Sprite m_background;
    sf::Sprite m_tabs;

    bool m_in;
    float m_scale;

    void buildHelp();
    void buildOptions();
    void buildCredits();
};

#endif //DC_MENU_STATE_HPP_