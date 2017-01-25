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

#include <MGPowerbar.hpp>

#include <xygine/util/Wavetable.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <vector>

namespace
{
    std::vector<float> wavetable;
    const float indicatorOffset = 4.f;
    const float indicatorWidth = 2.f;
}

Powerbar::Powerbar(const sf::Texture& t)
    : m_texture     (t),
    m_wavetableIdx  (0)
{
    if (wavetable.empty())
    {
        wavetable = xy::Util::Wavetable::sine(0.75f);
    }    
    
    sf::Vector2f texSize(t.getSize());
    m_vertices[1].position.x = texSize.x;
    m_vertices[1].texCoords.x = texSize.x;
    m_vertices[2].position = texSize;
    m_vertices[2].texCoords = texSize;
    m_vertices[3].position.y = texSize.y;
    m_vertices[3].texCoords.y = texSize.y;

    reset();

    setOrigin(texSize / 2.f);

    m_localBounds.width = texSize.x;
    m_localBounds.height = texSize.y;

    updateIndicator();
}

//public
void Powerbar::reset()
{
    m_wavetableIdx = 0;
    updateIndicator();
}

float Powerbar::getValue() const
{
    return wavetable[m_wavetableIdx];
}

void Powerbar::update(float dt)
{
    m_wavetableIdx = (m_wavetableIdx + 1) % wavetable.size();
    updateIndicator();
}

//private
void Powerbar::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &m_texture;

    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}

void Powerbar::updateIndicator()
{
    float offset = (wavetable[m_wavetableIdx] + 1.f) / 2.f;
    offset *= m_localBounds.width;
    
    m_vertices[4].position = { offset - 1.f, -indicatorOffset };
    m_vertices[5].position = { offset + indicatorWidth, -indicatorOffset };
    m_vertices[6].position = { offset + indicatorWidth, m_localBounds.height + indicatorOffset };
    m_vertices[7].position = { offset - 1.f, m_localBounds.height + indicatorOffset };
}