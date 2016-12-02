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

#ifndef DC_ATTRIB_MAN_HPP_
#define DC_ATTRIB_MAN_HPP_

#include <xygine/components/Component.hpp>

#include <array>

class AttribManager final : public xy::Component 
{
public:
    explicit AttribManager(xy::MessageBus&);
    ~AttribManager();

    xy::Component::Type type() const override { return xy::Component::Type::Script; }
    void entityUpdate(xy::Entity&, float) override;

    struct Personal final
    {
        enum
        {
            Health,
            Hunger,
            Thirst,
            Cleanliness,
            Tiredness,
            Poopiness,
            Boredness,
            Count
        };
    };

    struct Household
    {
        enum
        {
            Food,
            Water,
            Music,
            SheetMusic,
            Games,
            Films,
            IncomeRate,
            Count
        };
    };

    std::array<float, Personal::Count> getPersonalAttribs() const { return m_personalAttribs; }
    std::array<float, Household::Count> getHouseholdAttribs() const { return m_householdAttribs; }

private:

    std::array<float, Personal::Count> m_personalAttribs;
    std::array<float, Household::Count> m_householdAttribs;

    struct Stats final
    {
        std::int32_t currentIncome = 1000;
        std::int32_t daysToPayDay = 7;

        std::int64_t totalIncoming = 0;
        std::int64_t totalOutGoing = 0;
        std::uint64_t gameStartTime = 0;
    }m_stats;

    void initValues();
    void updateValues(float);
    void updateHealth();

    bool load();
    void save();

#ifdef _DEBUG_
        void addDebugWindow();
#endif //_DEBUG_
};

#endif //DC_ATTRIB_MAN_HPP_