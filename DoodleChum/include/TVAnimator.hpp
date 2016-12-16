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

#ifndef DC_TV_ANIMATOR_HPP_
#define DC_TV_ANIMATOR_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

class TVAnimator final : public xy::Component, public sf::Drawable
{
public: 
    TVAnimator(xy::MessageBus&);
    ~TVAnimator() = default;

    xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
    void entityUpdate(xy::Entity&, float) override;
    const sf::Texture& getMaskTexture() const { return m_maskTexture.getTexture(); }

private:

    mutable sf::RenderTexture m_maskTexture;
    sf::RectangleShape m_screenMask;
    bool m_on;
    bool m_prepped;
    std::int32_t m_animationCount;

    std::size_t m_wavetableIndex;
    sf::Color m_screenColour;

    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};

#endif //DC_TV_ANIMATOR_HPP_