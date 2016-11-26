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

#include <DayNightCycle.hpp>
#include <MessageIDs.hpp>

#include <xygine/Reports.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/util/Const.hpp>
#include <xygine/util/Math.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Font.hpp>

#include <sstream>
#include <ctime>

namespace
{
    const float totalSeconds = 24.f * 60.f * 60.f;
    const float sunset = xy::Util::Const::PI / 2.f;
    const float sunrise = sunset + xy::Util::Const::PI;
    const float fadeAngle = xy::Util::Const::TAU / 24.f;
    const float minIntensity = 0.2f;
    const float maxIntensity = 0.7f - minIntensity;

    const sf::Uint8 minBlue = 100;
    const sf::Uint8 minAmbience = 18;
    const sf::Uint8 maxAmbience = 80 - minAmbience;
}

DayNightCycle::DayNightCycle(xy::MessageBus& mb, xy::Scene::SkyLight& light, sf::Font& font, bool useSystemTime)
    : xy::Component(mb, this),
    m_rotation(0.f),
    m_intensity(1.f),
    m_lightColour(sf::Color::White),
    m_time(0.f),
    m_light(light)
{
    if (useSystemTime)
    {
        auto time = std::time(nullptr);
        auto tm = std::localtime(&time);
        m_time += tm->tm_sec;
        m_time += tm->tm_min * 60.f;
        auto hours = (tm->tm_hour > 12) ? tm->tm_hour - 12 : tm->tm_hour + 12;
        m_time += hours * 60.f * 60.f;
    }
    m_text.setFont(font);
    m_text.setFillColor(sf::Color::Red);
    updateText();
}

//public
void DayNightCycle::entityUpdate(xy::Entity& entity, float dt)
{
    m_time += dt/* * 1000.f*/;
    if (m_time > totalSeconds) m_time -= totalSeconds;

    m_rotation = ((1.f - (m_time / totalSeconds)) * xy::Util::Const::TAU);// +xy::Util::Const::PI;
    
    REPORT("rotation", std::to_string(m_rotation));

    float intensity = 0.f;
    if (m_rotation < xy::Util::Const::PI)
    {
        //afternoon
        intensity = xy::Util::Math::clamp((sunset - m_rotation) / fadeAngle, 0.f, 1.f);
    }
    else
    {
        //morning
        intensity = xy::Util::Math::clamp((m_rotation - sunrise) / fadeAngle, 0.f, 1.f);
    }
    m_intensity = (maxIntensity * intensity) + minIntensity;
    m_lightColour.r = static_cast<sf::Uint8>(255.f * intensity);
    m_lightColour.g = m_lightColour.r;
    m_lightColour.b = static_cast<sf::Uint8>(float(255 - minBlue) * intensity) + minBlue;

    sf::Uint8 ambience = static_cast<sf::Uint8>(float(maxAmbience) * intensity) + minAmbience;

    REPORT("intensity", std::to_string(intensity));

    m_light.setIntensity(m_intensity);
    m_light.setDiffuseColour(m_lightColour);
    entity.getScene()->setAmbientColour({ ambience, ambience, ambience });

    float dirX = std::sin(m_rotation);
    float dirY = std::cos(m_rotation);
    m_light.setDirection({ dirX, dirY, -1.2f  - dirY});
    REPORT("Y", std::to_string(dirY));

    updateText();

    auto msg = sendMessage<Message::TODEvent>(Message::TimeOfDay);
    msg->sunIntensity = intensity;
    msg->time = m_time;
}

//private
void DayNightCycle::updateText()
{
    //somehow we ended up with 00 as midday..
    int minutes = int(m_time / 60.f);
    int hours = minutes / 60;
    minutes = minutes % 60;

    hours = (hours > 12) ? hours - 12 : hours + 12;

    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << hours % 24 << ":"
        << std::setw(2) << std::setfill('0') << minutes;

    m_text.setString(ss.str());
}

void DayNightCycle::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_text, states);
}