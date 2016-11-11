/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

SuperTerrain - Zlib license.

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

#include <CameraController.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>

namespace
{
    const float maxDist = 480.f;
    const float maxDistSquare = maxDist * maxDist;
}

using namespace st;

CameraController::CameraController(xy::MessageBus& mb, xy::Entity& e)
    : xy::Component(mb, this),
    m_playerEntity(e)
{
    auto size = xy::DefaultSceneSize / 2.f;
    m_playerBounds.left = -(size.x / 2.f);
    m_playerBounds.top = -(size.y / 2.f);
    m_playerBounds.width = size.x;
    m_playerBounds.height = size.y;
}

//public
void CameraController::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_playerEntity.destroyed())
    {
        destroy();
    }
    else
    {
        auto bounds = entity.getWorldTransform().transformRect(m_playerBounds);
        auto playerPos = m_playerEntity.getWorldPosition();
        auto camPos = entity.getWorldPosition();
        if (!bounds.contains(playerPos))
        {
            sf::Vector2f diff
            (
                std::max(std::abs(playerPos.x - camPos.x) - bounds.width / 2.f, 0.f),
                std::max(std::abs(playerPos.y - camPos.y) - bounds.height / 2.f, 0.f)
            );
            entity.move(xy::Util::Vector::normalise(playerPos - camPos) * xy::Util::Vector::length(diff));
        }
    }   
}