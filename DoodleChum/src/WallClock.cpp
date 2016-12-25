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

#include <WallClock.hpp>

#include <xygine/util/Vector.hpp>
#include <xygine/SysTime.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    const float degsPerHour = 360.f / 12.f;
    const float degsPerMinute = 360.f / 60.f;
    const sf::Vector2f bigHand(0.f, -30.f);
    const sf::Vector2f smallHand(0.f, -18.f);
    const sf::Vector2f midPoint(1.8f, 0.f);
}

WallClock::WallClock(xy::MessageBus& mb, const sf::Texture& texture)
    : xy::Component(mb, this)
{
    sf::Color c(45, 43, 46);
    for (auto& v : m_handVertices)
    {
        v.color = c;
    }

    updateHands();

    m_globalBounds.left = bigHand.y;
    m_globalBounds.top = bigHand.y;
    m_globalBounds.width = -bigHand.y * 2.f;
    m_globalBounds.height = -bigHand.y * 2.f;

    setPosition(40.f, 154.f);
    setScale(1.f, -1.f);

    initSprite(texture);
}

//public
void WallClock::entityUpdate(xy::Entity& e, float dt)
{
    if (m_clock.getElapsedTime().asSeconds() > 60.f)
    {
        updateHands();
        m_clock.restart();
    }
    m_sprite->entityUpdate(e, dt);
}

//private
void WallClock::updateHands()
{
    const auto& time = xy::SysTime::now();
    float rotation = (degsPerHour * time.hours()) + ((static_cast<float>(time.minutes()) / 60.f) * degsPerHour);
    m_handVertices[1].position = xy::Util::Vector::rotate(smallHand, rotation);
    m_handVertices[2].position = xy::Util::Vector::rotate(midPoint, rotation);
    m_handVertices[0].position = -m_handVertices[2].position;
    
    rotation = degsPerMinute * time.minutes();
    m_handVertices[4].position = xy::Util::Vector::rotate(bigHand, rotation);
    m_handVertices[5].position = xy::Util::Vector::rotate(midPoint, rotation);
    m_handVertices[3].position = -m_handVertices[5].position;
}

void WallClock::initSprite(const sf::Texture& texture)
{
    m_sprite = xy::Component::create<xy::AnimatedDrawable>(getMessageBus(), texture);
    m_sprite->loadAnimationData("assets/images/sprites/clock.xya");
    
    auto frameSize = m_sprite->getFrameSize();
    m_texture.create(frameSize.x, frameSize.y);
    m_sprite->setScale(1.f, -1.f);
    m_sprite->setOrigin(0.f, static_cast<float>(frameSize.y));
    m_sprite->playAnimation(0);
}

void WallClock::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform = getTransform();
    m_texture.clear(sf::Color::Transparent);
    m_texture.draw(*m_sprite);
    m_texture.draw(m_handVertices.data(), m_handVertices.size(), sf::Triangles, states);
    m_texture.display();
}