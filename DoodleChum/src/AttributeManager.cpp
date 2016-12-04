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

#include <AttributeManager.hpp>

#include <xygine/App.hpp>
#include <xygine/imgui/imgui.h>
#include <xygine/Log.hpp>
#include <xygine/util/Random.hpp>

#include <iostream>
#include <cstring>
#include <ctime>

namespace
{
    const std::string filename("attrib.ute");
    const std::uint8_t fileVersion = 1;

    std::array<std::string, AttribManager::Personal::Count> personalNames = 
    {
        "Health",
        "Hunger",
        "Thirst",
        "Cleanliness",
        "Tiredness",
        "Poopiness",
        "Boredness"
    };

    std::array<std::string, AttribManager::Household::Count> householdNames =
    {
        "Food",
        "Water",
        "Music",
        "SheetMusic",
        "Games",
        "Films",
        "IncomeRate"
    };

    const int initialIncome = 1000;
    const int payPerWeek = 1000; //(multiplied by Houshold::Incomerate)
    const float minIncomeRate = 10.f;
    const int daysPerWeek = 7; //duh, but y'know

    const float hungerPerSecond = 0.0046f; //approx 100% every 6 hours
    const float thirstPerSecond = 0.028f; //approx 100% every hour
    const float cleanlinessPerSecond = 0.0023f; //approx every 12 hours
    const float tirednessPerSecond = 0.00154f; //approx 18 hours
    const float poopPerSecond = 0.00115f; //approx every 24 hours
    const float borednessPerSecond = 0.166667f; //approx every 10 minutes
}

AttribManager::AttribManager(xy::MessageBus& mb)
    : xy::Component (mb, this)
{
    initValues();
    
    //TODO register message handler to update attributes
    //both from actions performed and time (like pay)

#ifdef _DEBUG_
    addDebugWindow();
#endif //_DEBUG_
}

AttribManager::~AttribManager()
{
    xy::App::removeUserWindows(this);

    try
    {
        save();
    }
    catch (...) { std::cerr << "Failed writing attributes (exception thrown)" << std::endl; }
}

//public
void AttribManager::entityUpdate(xy::Entity&, float dt)
{
    updateValues(dt);
    updateHealth();
}

//private
void AttribManager::initValues()
{
    if (!load())
    {
        m_stats.currentIncome = initialIncome;
        m_stats.daysToPayDay = daysPerWeek;
        m_stats.gameStartTime = std::time(nullptr);
        m_stats.totalIncoming = 0;
        m_stats.totalOutGoing = 0;
        
        m_personalAttribs[Personal::Health] = 100.f;
        m_personalAttribs[Personal::Hunger] = xy::Util::Random::value(10.f, 25.f);
        m_personalAttribs[Personal::Thirst] = xy::Util::Random::value(15.f, 30.f);
        m_personalAttribs[Personal::Tiredness] = xy::Util::Random::value(12.f, 28.f);
        m_personalAttribs[Personal::Poopiness] = xy::Util::Random::value(10.f, 20.f);
        m_personalAttribs[Personal::Cleanliness] = 100.f;
        m_personalAttribs[Personal::Boredness] = xy::Util::Random::value(50.f, 70.f);

        m_householdAttribs[Household::Food] = 100.f;
        m_householdAttribs[Household::Water] = 100.f;
        m_householdAttribs[Household::Games] = 100.f;
        m_householdAttribs[Household::Music] = 100.f;
        m_householdAttribs[Household::SheetMusic] = 100.f;
        m_householdAttribs[Household::Films] = 100.f;
        m_householdAttribs[Household::IncomeRate] = 100.f;

        save();
    }
}

void AttribManager::updateValues(float dt)
{
    m_personalAttribs[Personal::Hunger] = std::min(100.f, m_personalAttribs[Personal::Hunger] + (hungerPerSecond * dt));
    m_personalAttribs[Personal::Thirst] = std::min(100.f, m_personalAttribs[Personal::Thirst] + (thirstPerSecond * dt));
    m_personalAttribs[Personal::Cleanliness] = std::max(0.f, m_personalAttribs[Personal::Cleanliness] - (cleanlinessPerSecond * dt));
    m_personalAttribs[Personal::Tiredness] = std::min(100.f, m_personalAttribs[Personal::Tiredness] + (tirednessPerSecond * dt));
    m_personalAttribs[Personal::Poopiness] = std::min(100.f, m_personalAttribs[Personal::Poopiness] + (poopPerSecond * dt));
    m_personalAttribs[Personal::Boredness] = std::min(100.f, m_personalAttribs[Personal::Boredness] + (borednessPerSecond * dt));
}

void AttribManager::updateHealth()
{
    float average =
        m_personalAttribs[Personal::Hunger] +
        m_personalAttribs[Personal::Thirst] +
        (100.f - m_personalAttribs[Personal::Cleanliness]) +
        m_personalAttribs[Personal::Tiredness] +
        m_personalAttribs[Personal::Poopiness] +
        m_personalAttribs[Personal::Boredness];
    m_personalAttribs[Personal::Health] = 100.f - (average / (Personal::Count - 1));

    if (m_personalAttribs[Personal::Health] < 1)
    {
        //DIED :(
    }

    //income rate drops with health
    m_householdAttribs[Household::IncomeRate] = minIncomeRate + ((100.f - minIncomeRate) * (m_personalAttribs[Personal::Health] / 100.f));
}

bool AttribManager::load()
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open() || !file.good())
    {
        xy::Logger::log("failed opening attrib settings file", xy::Logger::Type::Error, xy::Logger::Output::All);
        file.close();
        return false;
    }

    //format is:
    /*
    uint8 version
    float array personal attribs
    float array household attribs
    Stats stats struct
    uint64_t time since epoch
    */

    file.seekg(0, file.end);
    auto fileLength = file.tellg();
    file.seekg(0, file.beg);

    if ((int)fileLength <= 0) //TODO check file is expected size
    {
        xy::Logger::log("attrib file was empty", xy::Logger::Type::Error, xy::Logger::Output::All);
        file.close();
        return false;
    }

    std::vector<char> filedata(fileLength);
    file.read(filedata.data(), fileLength);
    file.close();

    auto* ptr = filedata.data();
    std::uint8_t version;
    std::memcpy(&version, ptr, sizeof(std::uint8_t));

    if (version != fileVersion)
    {
        xy::Logger::log("incorrect attrib file version", xy::Logger::Type::Error, xy::Logger::Output::All);
        return false;
    }

    ptr += sizeof(std::uint8_t);
    std::memcpy(m_personalAttribs.data(), ptr, sizeof(float) * Personal::Count);

    ptr += sizeof(float) * Personal::Count;
    std::memcpy(m_householdAttribs.data(), ptr, sizeof(float) * Household::Count);

    ptr += sizeof(float) * Household::Count;
    std::memcpy(&m_stats, ptr, sizeof(Stats));

    ptr += sizeof(Stats);
    std::uint64_t timeElapsed;
    std::memcpy(&timeElapsed, ptr, sizeof(std::uint64_t));

    //check time difference and update values, 5x slower than real time
    //rather than force a user to play at least once a day
    //make sure time difference is positive (could penalise people trying to cheat?)
    std::uint64_t timeNow = std::time(nullptr);
    auto diff = timeNow - timeElapsed;
    {
        if (diff <= 0)
        {
            //clock has been tampered with
            xy::Logger::log("invalid amount of time has passed, attributes have been reset...", xy::Logger::Type::Warning);
            return false;
        }
    }

    updateValues(static_cast<float>(diff) / 5.f); //time is 5 times slower when not played

    return true;
}

void AttribManager::save()
{
    //format is:
    /*
    uint8 version
    float array personal attribs
    float array household attribs
    Stats stats
    uint64 time since epoch
    */
    auto fileSize =
        sizeof(std::uint8_t) +
        (sizeof(float) * Personal::Count) +
        (sizeof(float) * Household::Count) +
        sizeof(Stats) +
        sizeof(std::uint64_t);
    std::vector<char> fileData(fileSize);

    auto* ptr = fileData.data();
    std::memcpy(ptr, &fileVersion, sizeof(std::uint8_t));

    ptr += sizeof(std::uint8_t);
    std::memcpy(ptr, m_personalAttribs.data(), sizeof(float) * Personal::Count);

    ptr += sizeof(float) * Personal::Count;
    std::memcpy(ptr, m_householdAttribs.data(), sizeof(float) * Household::Count);

    ptr += sizeof(float) * Household::Count;
    std::memcpy(&m_stats, ptr, sizeof(Stats));

    ptr += sizeof(Stats);
    std::uint64_t tse = std::time(nullptr);
    std::memcpy(ptr, &tse, sizeof(std::uint64_t));
    
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open() && file.good())
    {
        try
        {
            file.write(fileData.data(), fileSize);
        }
        catch (...)
        {
            xy::Logger::log("there was an exception writing to attrib file", xy::Logger::Type::Error, xy::Logger::Output::All);
        }
    }
}

#ifdef _DEBUG_
void AttribManager::addDebugWindow()
{
    xy::App::addUserWindow([this]() 
    {
        nim::SetNextWindowSize({ 220.f, 300.f });
        nim::Begin("Stats:");

        for (auto i = 0; i < Personal::Count; ++i)
        {
            nim::Text(std::string(personalNames[i] + ": " + std::to_string(m_personalAttribs[i])).c_str());
        }
        nim::Separator();

        for (auto i = 0; i < Household::Count; ++i)
        {
            nim::Text(std::string(householdNames[i] + ": " + std::to_string(m_householdAttribs[i])).c_str());
        }
        nim::End();
    }, this);
}
#endif //_DEBUG_