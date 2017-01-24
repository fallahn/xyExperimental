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

#include <MGDarts.hpp>
#include <AttributeManager.hpp>

#include <xygine/Resource.hpp>
#include <xygine/Entity.hpp>
#include <xygine/util/Position.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

DartsGame::DartsGame(xy::MessageBus& mb, xy::TextureResource& tr, AttribManager& am)
    : xy::Component(mb, this),
    m_textureResource(tr),
    m_attribManager(am),
    m_currentState(State::PlaceBet),
    m_powerbar(tr.get("assets/images/minigames/roulette/powerbar.png")),
    m_chanceSelector(tr.get("assets/images/minigames/roulette/credit_selector.png")),
    m_creditSelector(tr.get("assets/images/minigames/roulette/chance_selector.png")),
    m_reflection(tr.get("assets/images/ui/bob_screen.png"))
{
    m_powerbar.setPosition(0.f, 300.f);

    m_reflection.setScale(2.f, 2.f);
    xy::Util::Position::centreOrigin(m_reflection);
    m_reflection.setPosition(0.f, 66.f);

    xy::Util::Position::centreOrigin(m_chanceSelector);
    m_chanceSelector.setPosition(-280.f, 270.f);

    xy::Util::Position::centreOrigin(m_creditSelector);
    m_creditSelector.setPosition(280.f, 270.f);
}

//public
void DartsGame::entityUpdate(xy::Entity& entity, float dt)
{

}

//private
void DartsGame::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_powerbar, states);
    rt.draw(m_chanceSelector, states);
    rt.draw(m_creditSelector, states);

    switch (m_currentState)
    {
    default:break;
    case State::GameOver: break;
    }

    rt.draw(m_reflection, states);
}