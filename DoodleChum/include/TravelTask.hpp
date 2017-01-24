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

#ifndef DC_TRAVEL_TASK_HPP_
#define DC_TRAVEL_TASK_HPP_

#include <Task.hpp>

#include <SFML/System/Vector2.hpp>

#include <vector>

class TravelTask final : public Task
{
public:
    TravelTask(xy::Entity&, xy::MessageBus&, std::vector<sf::Vector2f>&);
    ~TravelTask() = default;

    void onStart() override;
    void update(float) override;

    Message::TaskEvent::Name getName() const override { return Message::TaskEvent::Travel; }

private:
    std::vector<sf::Vector2f> m_points;
    int m_currentAnimation;
    float m_moveSpeed;
    sf::Vector2f m_startDirection;

    void setAnimation(const sf::Vector2f&);
};

#endif //DC_TRAVEL_TASK_HPP_