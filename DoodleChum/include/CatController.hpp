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

#ifndef DC_CAT_CONTROLLER_HPP_
#define DC_CAT_CONTROLLER_HPP_

#include <Task.hpp>

#include <xygine/components/Component.hpp>
#include <xygine/components/AnimatedDrawable.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <list>

class PathFinder;
struct TaskData;
class CatController final : public xy::Component, public sf::Drawable
{
public:
    CatController(xy::MessageBus&, const PathFinder&, const std::vector<TaskData>&, const sf::Texture&);
    ~CatController() = default;

    xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
    void entityUpdate(xy::Entity&, float) override;
    void onStart(xy::Entity&) override;

    const sf::Texture& getTexture() const { return m_texture.getTexture(); }

private:
    xy::Entity* m_entity;
    const PathFinder& m_pathFinder;
    const std::vector<TaskData>& m_taskData;

    sf::Vector2u m_currentPosition;
    sf::Vector2u m_destinationPosition;

    std::list<Task::Ptr> m_tasks;
    void fillTaskStack();

    const sf::Texture& m_spriteSheet;
    xy::AnimatedDrawable::Ptr m_sprite;
    void initSprite();
    mutable sf::RenderTexture m_texture;
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};

#endif //DC_CAT_CONTROLLER_HPP_