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

#include <TrackRenderer.hpp>
#include <TrackData.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    const std::size_t MAX_VERTS = 100u;
}

TrackRenderer::TrackRenderer(xy::MessageBus& mb)
    : xy::Component(mb, this),
    m_vertexCount(0u)
{

}

//public
void TrackRenderer::entityUpdate(xy::Entity&, float)
{

}

void TrackRenderer::setData(const TrackData& td)
{
    m_vertexCount = std::min(MAX_VERTS, td.points.size());
    
    for (auto i = 0u; i < m_vertexCount; ++i)
    {
        m_vertices[i].position = td.points[i];
    }
}

//private
void TrackRenderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_vertices.data(), m_vertexCount, sf::PrimitiveType::LinesStrip, states);
}