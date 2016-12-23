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

#include <TVAnimator.hpp>
#include <MessageIDs.hpp>

#include <xygine/util/Wavetable.hpp>

namespace
{
    //TODO ideally we want to load this information
    //from the texture but for now lets assume the
    //screen is a rectangle 80,70,324,174 in the
    //main texture, and scale accordingly - NOTE Y inversion

    const sf::Vector2f screenPos(20.f, 195.f);
    const sf::Vector2f screenSize(81.f, 43.5f);

    std::vector<float> wavetable;

    const std::int32_t maxAnimations = 2;
}

TVAnimator::TVAnimator(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_on            (false),
    m_prepped       (false),
    m_animationCount(0),
    m_wavetableIndex(0u)
{
    m_maskTexture.create(256, 256);
    m_maskTexture.setSmooth(true);

    m_screenColour.r = 255;
    m_screenColour.g = 255;

    m_screenMask.setFillColor(m_screenColour);
    m_screenMask.setPosition(screenPos);
    m_screenMask.setSize(screenSize);

    if (wavetable.empty())
    {
        wavetable = xy::Util::Wavetable::sine(4.f, 0.2f);
        for (auto& w : wavetable)
        {
            w += 0.2f;
            w *= 0.5f;
        }
    }

    //ok this is a kludge because the music animation is the same
    //as the tv animation - and we only want to display the TV one...
    xy::Component::MessageHandler mh;
    mh.id = Message::NewTask;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::TaskEvent>();
        if (data.taskName == Message::TaskEvent::WatchTV)
        {
            m_prepped = true;
        }
        else
        {
            m_prepped = false;
        }
    };
    addMessageHandler(mh);

    mh.id = Message::Animation;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<Message::AnimationEvent>();
        if (data.id == Message::AnimationEvent::TV && m_prepped)
        {
            if (!m_on)
            {
                m_on = true;
                m_animationCount = maxAnimations;
            }
            else
            {
                //again a kludge, to ount the number of times animations are played
                //TODO make thi sa const somewhere so we don't go out of sync with
                //the task which raises these animations
                if (--m_animationCount == 0)
                {
                    m_on = false;
                }
            }
        }
    };
    addMessageHandler(mh);
}

//public
void TVAnimator::entityUpdate(xy::Entity&, float dt)
{
    static float fade = 0.f;
    
    if (m_on)
    {
        fade = std::min(1.f, fade + dt);
    }
    else
    {
        fade = std::max(0.f, fade - dt);
    }

    float intensity = 100.f - (55.f * wavetable[m_wavetableIndex]);
    m_screenColour.b = static_cast<sf::Uint8>(intensity * fade);
    m_screenMask.setFillColor(m_screenColour);
    m_wavetableIndex = (m_wavetableIndex + 1) % wavetable.size();
}

//private
void TVAnimator::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    m_maskTexture.clear();
    m_maskTexture.draw(m_screenMask);
    m_maskTexture.display();
}