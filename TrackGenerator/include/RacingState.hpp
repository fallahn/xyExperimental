/******************************************************************

Matt Marchant 2016
http://trederia.blogspot.com

xyRacer - Zlib license.

This software is provided 'as-is', without any express or
implied warranty.In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions :

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.

******************************************************************/

#ifndef XYR_RACING_STATE_HPP_
#define XYR_RACING_STATE_HPP_

#include <GameIDs.hpp>
#include <TrackSection.hpp>

#include <xygine/State.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Resource.hpp>
#include <xygine/ShaderResource.hpp>
#include <xygine/physics/World.hpp>
#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/mesh/MaterialResource.hpp>

class RacingState final : public xy::State
{
public:
    RacingState(xy::StateStack&, Context);
    ~RacingState() = default;

    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    bool update(float) override;
    void draw() override;

    xy::StateID stateID() const override { return StateID::Racing; }
private:

    xy::MessageBus& m_messageBus;
    xy::Scene m_scene;
    xy::Physics::World m_physWorld;
    xy::MeshRenderer m_meshRenderer;

    TrackSection m_trackSection;

    xy::TextureResource m_textureResource;
    xy::ShaderResource m_shaderResource;
    xy::MaterialResource m_materialResource;

    void init();
    void addBodies();
};

#endif //XYR_RACING_STATE_HPP_