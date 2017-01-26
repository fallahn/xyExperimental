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

#include <MGDartboard.hpp>

#include <xygine/util/Random.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

namespace
{
    const float gravity = 4.f;
}

Dartboard::Dart::Dart(const sf::Texture& t, sf::Vector2f position)
    : m_texture(t),
    m_rotationSpeed(xy::Util::Random::value(120.f, 180.f)),
    m_currentState(State::Ready),
    m_pauseTime(1.f)
{
    sf::Vector2f texSize(t.getSize());

    m_vertices[1].position.x = texSize.x;
    m_vertices[1].texCoords = m_vertices[1].position;
    m_vertices[2].position = texSize;
    m_vertices[2].texCoords = texSize;
    m_vertices[3].position.y = texSize.y;
    m_vertices[3].texCoords.y = m_vertices[3].position.y;

    setOrigin(texSize / 2.f);
    setPosition(position);
}

//public
void Dartboard::Dart::update(float dt, sf::Vector2f position)
{
    switch (m_currentState)
    {
    default: break;
    case State::Ready:
        setPosition(position);
        break;
    case State::InFlight:
    {
        rotate(m_rotationSpeed * dt);
        scale(0.97f, 0.97f);

        m_velocity.y += gravity;
        move(m_velocity * dt);

        if (getScale().x < 0.4f)
        {
            m_currentState = State::Landed;
        }
    }
        break;
    case State::Landed:
    {
        m_pauseTime -= dt;
        if (m_pauseTime < 0)
        {
            landedCallback(*this);
            m_currentState = State::Spent;
        }
    }
        break;
    case State::Spent:

        break;
    }
}

void Dartboard::Dart::fire(sf::Vector2f velocity)
{
    if (m_currentState == State::Ready)
    {
        m_velocity = velocity * 8.f;
        m_currentState = State::InFlight;
    }
}

void Dartboard::Dart::addCallback(const std::function<void(const Dart&)>& cb)
{
    landedCallback = cb;
}

//private
void Dartboard::Dart::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}