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

#include <ThinkBubble.hpp>
#include <MessageIDs.hpp>

#include <xygine/Entity.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

namespace
{
    sf::Vector2f offset(26.f, -170.f);
}

ThinkBubble::ThinkBubble(xy::MessageBus& mb, const sf::Texture& texture)
    : xy::Component (mb, this),
    m_sprite    (texture),
    m_scale     (1.f),
    m_grow      (true),
    m_entity    (nullptr)
{
    m_sprite.setOrigin(0.f, static_cast<float>(texture.getSize().y));
    m_sprite.setPosition(offset);

    xy::Component::MessageHandler mh;
    mh.id = Message::Animation;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::AnimationEvent>();

        if (data.id & Message::CatAnimMask) return; //ignore cat animations

        if (data.id == Message::AnimationEvent::Idle)
        {
            m_grow = true;
        }
        else
        {
            m_grow = false;
        }
    };
    addMessageHandler(mh);
}

void ThinkBubble::entityUpdate(xy::Entity& entity, float dt)
{
    dt *= 3.f;
    if (m_grow)
    {
        m_scale = std::min(1.f, m_scale + dt);
    }
    else
    {
        m_scale = std::max(0.f, m_scale - dt);
    }
    m_sprite.setScale(m_scale, m_scale);
}

//private
void ThinkBubble::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_sprite, states);
}