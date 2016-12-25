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

#ifndef DC_BUD_CONTROLLER_HPP_
#define DC_BUD_CONTROLLER_HPP_

#include <Task.hpp>

#include <xygine/components/Component.hpp>
#include <xygine/components/AnimatedDrawable.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <list>

class PathFinder;
struct TaskData;
class AttribManager;
class BudController final : public xy::Component, public sf::Drawable
{
public:
    BudController(xy::MessageBus&, const AttribManager&, const PathFinder&,
        const std::vector<TaskData>&, const std::vector<TaskData>&, const sf::Texture&);
    ~BudController();

    xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
    void entityUpdate(xy::Entity&, float) override;
    void onStart(xy::Entity&) override;
    void onDelayedStart(xy::Entity&) override;

    const sf::Texture& getTexture() const { return m_texture.getTexture(); }

private:
    xy::Entity* m_entity;
    const AttribManager& m_attribManager;
    const PathFinder& m_pathFinder;
    const std::vector<TaskData>& m_taskData;
    const std::vector<TaskData>& m_taskIdleData;

    sf::Vector2u m_currentPosition;
    sf::Vector2u m_destinationPosition;

    std::list<Task::Ptr> m_tasks;

    const sf::Texture& m_spriteSheet;
    xy::AnimatedDrawable::Ptr m_sprite; //unconventional but saves reinventing the wheel
    void initSprite();
    mutable sf::RenderTexture m_texture;
    void draw(sf::RenderTarget&, sf::RenderStates) const override;

#ifdef _DEBUG_
    void addConCommands();
#endif //_DEBUG_
};

#endif //DC_BUD_CONTROLLER_HPP_