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

#ifndef XYR_TRACK_DATA_HPP_
#define XYR_TRACK_DATA_HPP_

#include <xygine/Log.hpp>

#include <SFML/System/Vector2.hpp>

#include <vector>
#include <string>
#include <fstream>

static const sf::Vector2f MAX_AREA(21000.f, 21000.f);

struct TrackData final 
{
public:

    std::vector<sf::Vector2f> points;

    bool save(const std::string&) const;
    bool load(const std::string&);

private:

};

struct Parameters final
{
    int minPoints = 40;
    int maxPoints = 80;
    float minSegmentLength = 2.f;
    float maxSegmentLength = 16.f;
    float curviness = 0.3f;
    float maxAngle = 150.f;
    bool noCrossing = false;

    void save(const std::string& path) const
    {
        std::ofstream file(path, std::ios::binary);
        if (file.good() && file.is_open())
        {
            file.write((char*)this, sizeof(Parameters));
        }
        else
        {
            xy::Logger::log("Failed saving track gen parameters " + path, xy::Logger::Type::Error);
        }
        file.close();
    }

    void load(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (file.good() && file.is_open())
        {
            file.read((char*)this, sizeof(Parameters));
        }
        else
        {
            xy::Logger::log("Failed reading track gen parameters " + path, xy::Logger::Type::Error);
        }
        file.close();
    }
};

#endif //XYR_TRACK_DATA_HPP_