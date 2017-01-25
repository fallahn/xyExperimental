/*********************************************************************
Matt Marchant 2017
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

#ifndef DC_WHEEL_HPP_
#define DC_WHEEL_HPP_

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace sf
{
    class Texture;
}

namespace xy
{
    class TextureResource;
}

class Wheel final : public sf::Drawable, public sf::Transformable
{
public:
    explicit Wheel(xy::TextureResource&);
    ~Wheel() = default;

    void update(float);
    void spin(float);

    std::size_t getIndex(std::size_t segCount) const;

    bool stopped() const { return m_currentSpeed == 0; }

private:
    sf::Sprite m_wheelBack;
    sf::Sprite m_wheelFront;

    float m_currentSpeed;

    void draw(sf::RenderTarget&, sf::RenderStates) const;
};

#endif //DC_WHEEL_HPP_