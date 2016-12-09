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

#ifndef DC_TIME_TAB_HPP_
#define DC_TIME_TAB_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace xy
{
    class FontResource;
    class TextureResource;
}

class AttribManager;
class TimeTab final : public xy::Component, public sf::Drawable
{
public:
    TimeTab(xy::MessageBus&, xy::FontResource&, xy::TextureResource&, const AttribManager&);
    ~TimeTab() = default;

    xy::Component::Type type()const override { return xy::Component::Type::Drawable; }
    void entityUpdate(xy::Entity&, float) override;
    void onStart(xy::Entity& e) override { m_entity = &e; }

private:
    const AttribManager& m_attribManager;
    xy::Entity* m_entity;

    sf::Text m_titleText;

    sf::Text m_daysText;
    sf::Text m_balanceText;

    sf::Text m_clockShadow;
    sf::Text m_clockText;

    void updateClockText(float);

    sf::Sprite m_calendarSprite;
    sf::Sprite m_buttonSprite;

    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};

#endif //DC_TIME_TAB_HPP_