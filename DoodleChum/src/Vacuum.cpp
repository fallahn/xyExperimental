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

#include <Vacuum.hpp>

#include <xygine/Entity.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/util/Wavetable.hpp>

VacuumController::VacuumController(xy::MessageBus& mb)
    : xy::Component(mb, this),
    m_model(nullptr),
    m_index(0)
{
    m_waveTable = xy::Util::Wavetable::sine(0.6f, 15.f);
}

//public
void VacuumController::entityUpdate(xy::Entity&, float dt)
{
    m_model->setPosition({ m_waveTable[m_index], m_initialPosition.y, m_initialPosition.z });
    m_index = (m_index + 1) % m_waveTable.size();
}

void VacuumController::onStart(xy::Entity& entity)
{
    m_model = entity.getComponent<xy::Model>();
    XY_ASSERT(m_model, "entity has no model!");
    m_initialPosition = m_model->getTranslation();
}