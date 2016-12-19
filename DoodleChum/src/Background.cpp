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

#include <Background.hpp>
#include <MessageIDs.hpp>

#include <xygine/Entity.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace
{
    const sf::Uint8 minColour = 23;
    const float moveSpeed = 1.f;
    const float splitPoint = 3.f;
    const float cloudSplit = xy::DefaultSceneSize.y / splitPoint;
}

Background::Background(xy::MessageBus& mb, const sf::Texture& texture)
    : xy::Component(mb, this),
    m_texture(texture)
{
    m_vertices[1].position.x = xy::DefaultSceneSize.x;
    m_vertices[2].position = { xy::DefaultSceneSize.x, cloudSplit };
    m_vertices[3].position.y = cloudSplit;

    m_vertices[4].position.y = cloudSplit;
    m_vertices[5].position = { xy::DefaultSceneSize.x, cloudSplit };
    m_vertices[6].position = xy::DefaultSceneSize;
    m_vertices[7].position.y = xy::DefaultSceneSize.y;

    //tex coords
    auto texSize = static_cast<sf::Vector2f>(texture.getSize());
    const float texSplit = texSize.y / splitPoint;
    m_vertices[1].texCoords.x = texSize.x;
    m_vertices[2].texCoords = { texSize.x, texSplit };
    m_vertices[3].texCoords.y = texSplit;

    m_vertices[4].texCoords.y = texSplit;
    m_vertices[5].texCoords = { texSize.x, texSplit };
    m_vertices[6].texCoords = texSize;
    m_vertices[7].texCoords.y = texSize.y;

    xy::Component::MessageHandler mh;
    mh.id = Message::TimeOfDay;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::TODEvent>();
        sf::Uint8 sunlight = static_cast<sf::Uint8>((255.f - minColour) * data.sunIntensity);
        sf::Color c(sunlight, sunlight, sunlight);

        for (auto& v : m_vertices)
        {
            v.color = c;
        }

    };
    addMessageHandler(mh);
}

//public
void Background::entityUpdate(xy::Entity&, float dt)
{
    float amount = moveSpeed * dt;
    for (auto i = 0; i < 4; ++i)
    {
        m_vertices[i].texCoords.x += amount;
    }
}

//private
void Background::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}