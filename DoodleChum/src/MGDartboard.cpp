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

#include <xygine/util/Wavetable.hpp>
#include <xygine/util/Math.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Reports.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace
{
    const sf::Color crosshairColour(128, 128, 128, 130);
    const float crosshairThickness = 2.f;
}

Dartboard::Dartboard(const sf::Texture& t, const sf::Texture& dt)
    : m_texture     (t),
    m_dartTexture   (dt),
    m_xIdx          (0),
    m_yIdx          (0),
    m_ampIdx        (0),
    m_score         (0u),
    m_remainingDarts(3u)
{
    m_boardSize = static_cast<sf::Vector2f>(t.getSize());
    setOrigin(m_boardSize / 2.f);

    m_vertices[1].position.x = m_boardSize.x;
    m_vertices[1].texCoords.x = m_boardSize.x;
    m_vertices[2].position = m_boardSize;
    m_vertices[2].texCoords = m_boardSize;
    m_vertices[3].position.y = m_boardSize.y;
    m_vertices[3].texCoords.y = m_boardSize.y;

    showCrosshair(false);

    m_xOffsets = xy::Util::Wavetable::sine(1.72f, 1.2f);
    m_yOffsets = xy::Util::Wavetable::sine(2.28f, 2.7f);
    m_amplitudeModifier = xy::Util::Wavetable::sine(0.34f);

    m_darts.reserve(m_remainingDarts);
}

//public
void Dartboard::update(float dt, sf::Vector2f mousePos)
{
    m_xIdx = (m_xIdx + 1) % m_xOffsets.size();
    m_yIdx = (m_yIdx + 1) % m_yOffsets.size();
    m_ampIdx = (m_ampIdx + 1) % m_amplitudeModifier.size();
    
    m_lastPos = m_mousePos;
    m_mousePos = getInverseTransform().transformPoint(mousePos);
    m_mousePos.x += m_xOffsets[m_xIdx] + m_amplitudeModifier[m_ampIdx];
    m_mousePos.y += m_yOffsets[m_yIdx] + m_amplitudeModifier[m_ampIdx];
    m_mousePos.x = xy::Util::Math::clamp(m_mousePos.x, 0.f, m_boardSize.x);
    m_mousePos.y = xy::Util::Math::clamp(m_mousePos.y, 0.f, m_boardSize.y);

    //crosshair X
    m_vertices[4].position.x = m_mousePos.x - crosshairThickness;
    m_vertices[5].position.x = m_mousePos.x + crosshairThickness;
    m_vertices[6].position = { m_vertices[5].position.x, m_boardSize.y };
    m_vertices[7].position = { m_vertices[4].position.x, m_boardSize.y };

    m_vertices[8].position.y = m_mousePos.y - crosshairThickness;
    m_vertices[9].position = { m_boardSize.x, m_mousePos.y - crosshairThickness };
    m_vertices[10].position = { m_boardSize.x, m_mousePos.y + crosshairThickness };
    m_vertices[11].position.y = m_mousePos.y + crosshairThickness;


    //update darts
    for (auto& d : m_darts) d.update(dt, m_mousePos);
    if (m_pendingDart)
    {
        m_pendingDart = false;
        addDart();
    }
    //REPORT("Dart Pos", std::to_string(m_mousePos.x) + ", " + std::to_string(m_mousePos.y));
}

void Dartboard::fire()
{
    m_darts.back().fire(m_mousePos - m_lastPos);
}

void Dartboard::showCrosshair(bool show)
{
    for (auto i = 4u; i < m_vertices.size(); ++i)
    {
        m_vertices[i].color = (show) ? crosshairColour : sf::Color::Transparent;
        m_vertices[i].texCoords = { m_boardSize.x / 2.f, 4.f }; //just try and grab a white bit of texture
    }
    m_pendingDart = show;
}

//private
void Dartboard::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
    for (const auto& d : m_darts)
    {
        rt.draw(d, states);
    }
}

void Dartboard::addDart()
{
    m_darts.emplace_back(m_dartTexture, m_mousePos);
    m_darts.back().addCallback([this](const Dartboard::Dart& dart)
    {
        //get score from position
        sf::Vector2f centre = m_boardSize / 2.f;
        float distance = xy::Util::Vector::length(dart.getPosition() - centre);
        if (distance < centre.x)
        {
            float ratio = 1.f - (distance / centre.x);
            std::uint32_t score = static_cast<std::uint32_t>(/*std::ceil*/(9.f * ratio)) + 1;
            m_score += score;

         //   std::cout << score << std::endl;
        }
        m_remainingDarts--;
        if (m_remainingDarts > 0) m_pendingDart = true;
    });
}