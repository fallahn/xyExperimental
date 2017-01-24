/*********************************************************************
Matt Marchant 2017
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

#ifndef DC_DARTS_HPP_
#define DC_DARTS_HPP_

#include <MGPowerbar.hpp>
#include <MGSelector.hpp>

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace xy
{
    class TextureResource;
}

class AttribManager;
class DartsGame final : public xy::Component, public sf::Drawable
{
public:
    DartsGame(xy::MessageBus&, xy::TextureResource&, AttribManager&);
    ~DartsGame() = default;

    xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
    void entityUpdate(xy::Entity&, float) override;
    sf::FloatRect globalBounds() const override { return{ 0.f, 0.f, 1920.f, 1080.f }; }

private:

    xy::TextureResource& m_textureResource;
    AttribManager& m_attribManager;

    enum class State
    {
        PlaceBet, Charging, Shooting, Summary, GameOver
    }m_currentState;

    Powerbar m_powerbar;
    Selector m_chanceSelector;
    Selector m_creditSelector;
    sf::Sprite m_reflection;

    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};

#endif //DC_DARTS_HPP_