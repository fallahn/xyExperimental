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

#include <Rain.hpp>
#include <MessageIDs.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

namespace
{
    const float rainSpeed = -550.f;
    const sf::Uint8 minColour = 23;
}

RainEffect::RainEffect(xy::MessageBus& mb, sf::Texture& t)
    : xy::Component(mb, this),
    m_texture       (t),
    m_transparency  (0.f),
    m_stopped       (true)
{
    m_texture.setRepeated(true);

    m_vertices[1].position.x = xy::DefaultSceneSize.x;
    m_vertices[2].position = xy::DefaultSceneSize;
    m_vertices[3].position.y = xy::DefaultSceneSize.y;

    for (auto i = 0; i < 4; ++i)
    {
        m_vertices[i + 4].position = m_vertices[i].position;
    }

    //TODO shorten the Y value of bottom coords to stretch
    //the texture making it look like it's falling faster near the bottom?
    for (auto& v : m_vertices) v.texCoords = v.position;


    xy::Component::MessageHandler mh;
    mh.id = Message::Weather;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        m_stopped = !msg.getData<bool>();
    };
    addMessageHandler(mh);

    mh.id = Message::TimeOfDay;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::TODEvent>();
        sf::Uint8 sunlight = static_cast<sf::Uint8>((235.f - minColour) * data.sunIntensity) + 20;

        for (auto& v : m_vertices)
        {
            v.color.r = v.color.g = v.color.b = sunlight;
        }

    };
    addMessageHandler(mh);
}

//public
void RainEffect::entityUpdate(xy::Entity&, float dt)
{
    if (m_stopped)
    {
        m_transparency = std::max(0.f, m_transparency - (dt / 2.f));
    }
    else
    {
        m_transparency = std::min(1.f, m_transparency + (dt / 2.f));
    }
    sf::Uint8 alpha = static_cast<sf::Uint8>(255.f * m_transparency);
    
    if (m_transparency > 0)
    {
        float speed = rainSpeed * dt;
        for (auto i = 0; i < 4; ++i)
        {
            m_vertices[i].texCoords.y += speed;
            m_vertices[i].color.a = alpha;
        }
        speed *= 1.5f;
        for (auto i = 4; i < 8; ++i)
        {
            m_vertices[i].texCoords.y += speed;
            m_vertices[i].color.a = alpha;
        }
    }
}

//private
void RainEffect::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if (m_transparency > 0)
    {
        states.texture = &m_texture;
        //states.blendMode = sf::BlendAdd;
        rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
    }
}