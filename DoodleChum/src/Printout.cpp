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

#include <Printout.hpp>

#include <xygine/Resource.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace
{
    const float scrollSpeed = 100.f;
    const sf::Vector2f defaultTextPos(18.f, 230.f);
    const std::size_t charLimit = 26;
}

Printout::Printout(sf::Font& font, xy::TextureResource& tr)
    : m_stringIdx   (0),
    m_texture       (nullptr),
    m_scrollDistance(0.f)
{
    m_texture = &tr.get("assets/images/ui/printer.png");
    m_texture->setRepeated(true);

    sf::Vector2f textureSize = static_cast<sf::Vector2f>(m_texture->getSize());
    m_vertices[1].position.x = textureSize.x / 2.f;
    m_vertices[1].texCoords.x = textureSize.x / 2.f;
    m_vertices[2].position = { textureSize.x / 2.f, textureSize.y };
    m_vertices[2].texCoords = m_vertices[2].position;
    m_vertices[3].position.y = textureSize.y;
    m_vertices[3].texCoords.y = textureSize.y;

    m_vertices[4].texCoords.x = textureSize.x / 2.f;
    m_vertices[5].position.x = m_vertices[1].position.x;
    m_vertices[5].texCoords.x = textureSize.x;
    m_vertices[6].position = m_vertices[2].position;
    m_vertices[6].texCoords = textureSize;
    m_vertices[7].position = m_vertices[3].position;
    m_vertices[7].texCoords = m_vertices[2].texCoords;

    m_text.setFillColor(sf::Color::Black);
    m_text.setFont(font);
    m_text.setPosition(defaultTextPos);
    m_text.setCharacterSize(24u);

    m_textTexture.create(m_texture->getSize().x / 2, m_texture->getSize().y);
    m_textSprite.setTexture(m_textTexture.getTexture());

    m_borderSprite.setTexture(tr.get("assets/images/ui/pen_border.png"));
    m_borderSprite.setPosition(-8.f, -8.f);
}

//public
void Printout::printLine(const std::string & str)
{
    //split at nearest space under max char size
    //or at max char size if no space is found.
    if (str.size() >= charLimit)
    {
        auto searchStr = str;
        while (searchStr.size() >= charLimit)
        {
            auto split = searchStr.find_last_of(' ', charLimit);
            if (split == std::string::npos)
            {
                split = charLimit;
            }
            m_strings.push_back(searchStr.substr(0, split));
            searchStr = searchStr.substr(split + 1);
        }
        m_strings.push_back(searchStr);
    }
    else
    {
        m_strings.push_back(str);
    }

    Task task = [this](float dt)->bool
    {
        if (!m_strings.empty())
        {
            scroll(dt);
            
            if (m_scrollDistance == 0)
            {
                auto str = m_text.getString();
                str += m_strings.front()[m_stringIdx++];

                if (m_stringIdx == m_strings.front().size())
                {
                    m_strings.pop_front();
                    str += '\n';
                    m_stringIdx = 0;
                    m_scrollDistance = m_text.getFont()->getLineSpacing(m_text.getCharacterSize());
                }
                m_text.setString(str);
            }
        }

        return m_strings.empty();
    };
    m_tasks.emplace_back(std::move(task));
}

void Printout::update(float dt)
{
    if (!m_tasks.empty())
    {
        if (m_tasks.front()(dt))
        {
            m_tasks.pop_front();
        }
    }
}

void Printout::clear()
{
    m_scrollDistance = m_text.getGlobalBounds().top + m_text.getGlobalBounds().height;
    Task task = [this](float dt)->bool
    {
        scroll(dt);
        if (m_scrollDistance == 0)
        {
            m_text.setPosition(defaultTextPos);
            m_text.setString("");
            return true;
        }

        return false;
    };
    m_tasks.emplace_back(std::move(task));
}

//private
void Printout::scroll(float dt)
{
    if (m_scrollDistance > 0)
    {
        float dist = scrollSpeed * dt;
        m_vertices[0].texCoords.y += dist;
        m_vertices[1].texCoords.y += dist;
        m_vertices[2].texCoords.y += dist;
        m_vertices[3].texCoords.y += dist;

        m_text.move(0.f, -dist);

        m_scrollDistance -= dist;
        if (m_scrollDistance < 0)
        {
            m_vertices[0].texCoords.y += m_scrollDistance;
            m_vertices[1].texCoords.y += m_scrollDistance;
            m_vertices[2].texCoords.y += m_scrollDistance;
            m_vertices[3].texCoords.y += m_scrollDistance;
            m_text.move(0.f, -m_scrollDistance);

            m_scrollDistance = 0;
        }
    }
}

void Printout::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    m_textTexture.clear(sf::Color::Transparent);
    m_textTexture.draw(m_text);
    m_textTexture.display();
    
    states.transform *= getTransform();
    states.texture = m_texture;

    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);

    states.texture = nullptr;

    rt.draw(m_textSprite, states);
    rt.draw(m_borderSprite, states);
}