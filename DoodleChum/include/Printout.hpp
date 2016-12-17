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

#ifndef DC_PRINTOUT_HPP_
#define DC_PRINTOUT_HPP_

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <array>
#include <functional>
#include <list>

namespace sf
{
    class RenderWindow;
}

class Printout final : public sf::Drawable, public sf::Transformable
{
public:
    Printout(sf::Font&, const sf::Texture&);
    ~Printout() = default;
    Printout(const Printout&) = delete;
    Printout& operator = (const Printout&) = delete;

    void printLine(const std::string&);
    void update(float);
    void clear();

public:

    using Task = std::function<bool(float)>;
    std::list<Task> m_tasks;
    std::list<std::string> m_strings;
    std::size_t m_stringIdx; //char index in to current string

    const sf::Texture& m_texture;
    sf::Text m_text;

    float m_scrollDistance;
    void scroll(float);

    mutable sf::RenderTexture m_textTexture;
    sf::Sprite m_textSprite;

    std::array<sf::Vertex, 8u> m_vertices;
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};


#endif //DC_PRINTOUT_HPP_