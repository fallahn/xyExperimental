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

#ifndef DC_WORLDCLIENT_STATE_HPP_
#define DC_WORLDCLIENT_STATE_HPP_

#include <StateIDs.hpp>
#include <PathFinder.hpp>
#include <Task.hpp>
#include <AttributeManager.hpp>

#include <xygine/State.hpp>
#include <xygine/Scene.hpp>
#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/Resource.hpp>
#include <xygine/ShaderResource.hpp>
#include <xygine/mesh/MaterialResource.hpp>
#include <xygine/physics/World.hpp>

class WorldClientState final : public xy::State
{
public:
    WorldClientState(xy::StateStack&, Context);
    ~WorldClientState() = default;

    bool update(float) override;
    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    void draw() override;

    xy::StateID stateID() const override { return States::WorldClient; }

private:
    xy::MessageBus& m_messageBus;
    xy::Scene m_scene;
    xy::MeshRenderer m_meshRenderer;
    xy::Physics::World m_physWorld;

    xy::TextureResource m_textureResource;
    xy::ShaderResource m_shaderResource;
    xy::FontResource m_fontResource;
    xy::SoundResource m_soundResource;

    PathFinder m_pathFinder;
    std::vector<TaskData> m_tasks;
    std::vector<TaskData> m_idleTasks;
    std::vector<TaskData> m_catTasks;

    AttribManager m_attribManager;

    //TODO this should be kept elsewhere
    void updateWeather(float);

    void initMeshes();
    void initMapData();
    void initBud();
    void initCat();
    void initParticles();
    void initUI();
    void initMiniGame();
    void initSounds();

    //minigame creators
    void createRoulette();
    void createDarts();
    void createPachinko();

    sf::Sprite m_loadingSprite;
    void updateLoadingScreen(float, sf::RenderWindow&) override;
};

#endif //DC_WORLD_CLIENT_STATE_HPP_