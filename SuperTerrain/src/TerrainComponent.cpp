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

#include <TerrainComponent.hpp>

#include <xygine/detail/GLExtensions.hpp>
#include <xygine/detail/GLCheck.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/App.hpp>
#include <xygine/FileSystem.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/util/Math.hpp>

#include <xygine/imgui/imgui.h>

#include <FastNoiseSIMD.h>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    const std::string vertex =
        "#version 120\n"

        "varying vec2 v_texCoord;\n"
        "varying vec4 v_colour;\n"

        "void main()\n"
        "{\n"
        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
        "    v_texCoord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;"
        "    v_colour = gl_Color;\n"
        "}";

    const std::string waterShader =
        "#version 130\n"
        "uniform sampler2D u_floorTexture;\n"
        "uniform usampler2D u_depthTexture;\n"
        "uniform sampler2D u_reflectionTexture;\n"
        "uniform float u_time = 0.0;\n"
        "uniform vec2 u_screenSize = vec2(800, 600);\n"

        "in vec2 v_texCoord;\n"
        "out vec4 colour;\n"

        "const float TAU = 6.28318;\n"
        "const float tileFrequency = 0.35;"

        "void main()\n"
        "{\n"
        "    vec2 texSize = vec2(textureSize(u_floorTexture, 0));\n"
        "    vec2 texCoord = v_texCoord * texSize * 2.0;\n"
        "    float valueX = sin(v_texCoord.x * TAU * tileFrequency * texSize.x) * 0.5 + 0.5;\n"
        "    valueX = sin((u_time * 10.0) + (valueX * TAU)) * 0.5 + 0.5;\n"

        "    float valueY = sin(v_texCoord.y * TAU * tileFrequency * texSize.y) * 0.5 + 0.5;\n"
        "    valueY = sin((u_time * 12.0) + (valueY * TAU)) * 0.5 + 0.5;\n"
        "    vec2 offset = vec2(valueX, valueY);\n"

        "    vec4 reflection = texture(u_reflectionTexture, (gl_FragCoord.xy / u_screenSize) + offset / 100.0);\n"

        "    float depth = float((texture(u_depthTexture, v_texCoord).r & 0xF000u) >> 12) / 15.0;\n"
        "    colour = vec4(texture(u_floorTexture, texCoord + (offset / 12.0)).rgb * depth, 1.0);\n"
        "    colour += reflection * 0.15;\n"
        "}";

    const std::string tileShader =
        "#version 130\n"

        "uniform usampler2D u_texture;\n"
        "uniform int u_output = 0;"

        "in vec2 v_texCoord;\n"
        "in vec4 v_colour;\n"

        "out vec4 colour;\n"

        "vec3[4] colours = vec3[4](vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 0.0), vec3(0.8, 1.0, 0.0), vec3(0.0, 0.9, 0.05));\n"

        "vec3[9] biomes = vec3[9](vec3(0.7,0.87,0.93), vec3(0.93,0.85,0.7), vec3(0.58,0.41,0.12), vec3(0.89,0.35,0.23), vec3(0.63,0.81,0.1), vec3(0.22,0.72,0.55), vec3(0.22, 0.5, 0.72), vec3(0.14,0.33,0.66), vec3(0.04,0.32,0.06));\n"

        "void main()\n"
        "{\n"
        "    uint value = texture(u_texture, v_texCoord).r;\n"
        "    //colour = vec4(vec3(float(value) / 65535.0) * v_colour.rgb, 1.0);\n"
        "    if(u_output == 0)\n"
        "    {\n"
        "        float alpha = clamp(float(value & 0xFFu), 0.0, 1.0);"
        "        //float depth = float((value & 0xF000u) >> 12) / 15.0;\n"
        "        colour = vec4(colours[(value & 0xFFu)], alpha);\n"
        "        return;\n"
        "    }\n"
        "    if(u_output == 1)\n"
        "    {\n"
        "        colour = vec4(colours[(value & 0xFFu)] * 0.8, 1.0);\n"
        "    }\n"
        "    else if(u_output == 2)\n"
        "    {\n"
        "        if((value & 0xFFu) == 0u)\n"
        "        {\n"
        "            colour = vec4(colours[0], 1.0)\n;"
        "        }\n"
        "        else\n"
        "        {\n"
        "            colour = vec4(biomes[(value & 0x0F00u) >> 8], 1.0);\n"
        "        }\n"
        "    }\n"
        "    else if(u_output == 3)\n"
        "    {\n"
        "        colour = vec4(biomes[(value & 0x0F00u) >> 8] * colours[(value & 0xFFu)], 1.0);\n"
        "    }\n"
        "}";

    const std::size_t maxActiveChunks = 18;

    const std::array<sf::FloatRect, 3u> viewPorts =
    {
        sf::FloatRect(0.56f, 0.05f, 0.54f, 0.3f),
        sf::FloatRect(0.56f, 0.37f, 0.54f, 0.3f),
        sf::FloatRect(0.56f, 0.68f, 0.54f, 0.3f)
    };
    sf::View miniView(sf::Vector2f(), xy::DefaultSceneSize * 16.f);

    int seed = 12345;
}

TerrainComponent::TerrainComponent(xy::MessageBus& mb)
    :xy::Component  (mb, this),
    m_maxDistance   (xy::Util::Vector::lengthSquared(Chunk::chunkSize() * 1.7f)),
    m_texturePool   (maxActiveChunks),
    m_currentChunk  (nullptr),
    m_chunkPool     (maxActiveChunks)
{
    //radial points used for testing surrounding chunks
    auto length = xy::Util::Vector::length(Chunk::chunkSize());
    m_radialPoints =
    {
        std::make_pair(sf::Vector2f(0.f, 1.f) * length, false),
        std::make_pair(xy::Util::Vector::normalise({-1.f, 1.f}) * length, false),
        std::make_pair(sf::Vector2f(-1.f, 0.f) * length, false),
        std::make_pair(xy::Util::Vector::normalise({ -1.f, -1.f }) * length, false),
        std::make_pair(sf::Vector2f(0.f, -1.f) * length, false),
        std::make_pair(xy::Util::Vector::normalise({ 1.f, -1.f }) * length, false),
        std::make_pair(sf::Vector2f(1.f, 0.f) * length, false),
        std::make_pair(xy::Util::Vector::normalise({ 1.f, 1.f }) * length, false)
    };

    m_waterShader.loadFromMemory(vertex, waterShader);
    m_waterFloorTexture.loadFromFile("assets/images/tiles/water_tile.png");
    m_waterFloorTexture.setRepeated(true);
    m_waterShader.setUniform("u_floorTexture", m_waterFloorTexture);
    updateReflectionTexture();

    xy::Component::MessageHandler mh;
    mh.id = xy::Message::UIMessage;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        auto msgData = msg.getData<xy::Message::UIEvent>();
        if (msgData.type == xy::Message::UIEvent::ResizedWindow)
        {
            m_waterShader.setUniform("u_screenSize", sf::Glsl::Vec2(1920.f, 1080.f)); //TODO get the proper value
        }
    };
    addMessageHandler(mh);

    m_terrainShader.loadFromMemory(vertex, tileShader);

    //set up the texture pool
    for (auto& tp : m_texturePool)
    {
        tp.first.create(Chunk::chunkTilesSide(), Chunk::chunkTilesSide());
        glCheck(glBindTexture(GL_TEXTURE_2D, tp.first.getNativeHandle()));
        glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI, tp.first.getSize().x, tp.first.getSize().y, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, 0));
        //glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16UI, tp.first.getSize().x, tp.first.getSize().y, 0, GL_RGB_INTEGER, GL_UNSIGNED_SHORT, 0));
        glCheck(glBindTexture(GL_TEXTURE_2D, 0));
        tp.second = false; //texture not yet used
    }

    //check we have a directory to write chunk data to
    if (!xy::FileSystem::directoryExists("map"))
    {
        xy::FileSystem::createDirectory("map");
    }

#ifdef _DEBUG_
    registerWindow();
#endif //_DEBUG_
}

TerrainComponent::~TerrainComponent()
{
    xy::App::removeUserWindows(this);
}

//public
void TerrainComponent::entityUpdate(xy::Entity& entity, float dt)
{
    m_playerPosition = entity.getScene()->getView().getCenter();

    auto result = std::find_if(std::begin(m_activeChunks), std::end(m_activeChunks),
        [this](const ChunkPtr& chunk)
    {
        return chunk->getGlobalBounds().contains(m_playerPosition);
    });

    bool update = false;
    if (result == m_activeChunks.end() || !m_currentChunk)
    {
        //generate or load new chunk at position
        sf::Vector2f chunkPos = 
        {
            std::floor(m_playerPosition.x / Chunk::chunkSize().x) * Chunk::chunkSize().x,
            std::floor(m_playerPosition.y / Chunk::chunkSize().y) * Chunk::chunkSize().y
        };
        chunkPos += (Chunk::chunkSize() / 2.f);
        m_activeChunks.emplace_back(m_chunkPool.get(chunkPos, m_terrainShader, getTexture(), m_waterShader, m_waterFloorTexture));

        //set current chunk to new chunk
        m_currentChunk = m_activeChunks.back().get();

        update = true;
    }
    else if (result->get() != m_currentChunk)
    {
        //we moved to an existing chunk
        m_currentChunk = result->get();
        update = true;
    }
    if (update) updateChunks();

    //this updates any pending texture changes
    for (auto& c : m_activeChunks) c->update();

    //update the water shader
    static float waterTime = dt;
    waterTime += dt;
    m_waterShader.setUniform("u_time", waterTime);
}

int TerrainComponent::getSeed() { return seed; }
void TerrainComponent::setSeed(int s) { seed = s; }

//private
void TerrainComponent::updateReflectionTexture()
{
    auto noise = FastNoiseSIMD::NewFastNoiseSIMD(seed);
    noise->SetFrequency(0.005f);
    noise->SetFractalType(FastNoiseSIMD::FBM);
    noise->SetFractalOctaves(3);
    noise->SetFractalLacunarity(2.f);

    static const int texSize = 480;
    float* noiseData = noise->GetValueFractalSet(0, 0, 0, 2, texSize, texSize, 5.f);
    std::vector<sf::Uint8> texData(texSize * texSize * 4);
    
    for (auto z = 0; z < texSize; ++z)
    {
        for (auto y = 0; y < texSize; ++y)
        {
            auto index = z * texSize + y;
            std::size_t i = index * 4;

            int value = static_cast<int>((noiseData[index] * 0.5 + 0.5) * 255.f);
            texData[i++] = xy::Util::Math::clamp(value + 14, 0, 255);
            texData[i++] = xy::Util::Math::clamp(value + 45, 0, 255);
            texData[i++] = xy::Util::Math::clamp(value + 150, 0, 255);
            texData[i++] = 255;
        }
    }

    FastNoiseSIMD::FreeNoiseSet(noiseData);

    sf::Image img;
    img.create(texSize, texSize, texData.data());

    m_waterReflectionTexture.create(texSize, texSize);
    m_waterReflectionTexture.loadFromImage(img);
    m_waterReflectionTexture.setSmooth(true);
    m_waterShader.setUniform("u_reflectionTexture", m_waterReflectionTexture);
}

ChunkTexture& TerrainComponent::getTexture()
{
    return *std::find_if(std::begin(m_texturePool), std::end(m_texturePool), 
        [](const ChunkTexture& ct)
    {
        return !ct.second;
    });
}

void TerrainComponent::updateChunks()
{
    //move around current chunk and delete as necessary
    for (auto& chunk : m_activeChunks)
    {
        if (xy::Util::Vector::lengthSquared(m_currentChunk->getPosition() - chunk->getPosition()) >
            m_maxDistance)
        {
            chunk->destroy();
        }
    }

    //tidy up dead chunks
    m_activeChunks.erase(std::remove_if(std::begin(m_activeChunks), std::end(m_activeChunks), 
        [](const ChunkPtr& chunk)
    {
        return chunk->destroyed();
    }), std::end(m_activeChunks));


    //TODO this could be optimised - so many loops!
    for (auto& point : m_radialPoints) point.second = false;
    auto currentChunkPos = m_currentChunk->getPosition();
    for (const auto& chunk : m_activeChunks)
    {        
        for (auto& point : m_radialPoints)
        {
            if (!point.second && chunk->getGlobalBounds().contains(point.first + currentChunkPos))
            {
                point.second = true;
                break;
            }
        }
    }

    for (const auto& point : m_radialPoints)
    {
        if (!point.second)
        {
            auto currentPos = point.first + currentChunkPos;
            sf::Vector2f chunkPos =
            {
                std::floor(currentPos.x / Chunk::chunkSize().x) * Chunk::chunkSize().x,
                std::floor(currentPos.y / Chunk::chunkSize().y) * Chunk::chunkSize().y
            };
            chunkPos += (Chunk::chunkSize() / 2.f); //account for positioning being in centre
            m_activeChunks.emplace_back(m_chunkPool.get(chunkPos, m_terrainShader, getTexture(), m_waterShader, m_waterFloorTexture));
        }
    }
}

void TerrainComponent::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    m_terrainShader.setUniform("u_output", 0);
    //states.shader = &m_shader;
    for (const auto& chunk : m_activeChunks)
    {
        rt.draw(*chunk, states);
    }

#ifdef _DEBUG_

    //draw the mini views
    auto oldView = rt.getView();
    miniView.setCenter(oldView.getCenter());
    
    m_terrainShader.setUniform("u_output", 1);
    miniView.setViewport(viewPorts[0]);
    rt.setView(miniView);
    for (const auto& chunk : m_activeChunks)
    {
        rt.draw(*chunk, states);
    }

    m_terrainShader.setUniform("u_output", 2);
    miniView.setViewport(viewPorts[1]);
    rt.setView(miniView);
    for (const auto& chunk : m_activeChunks)
    {
        rt.draw(*chunk, states);
    }

    m_terrainShader.setUniform("u_output", 3);
    miniView.setViewport(viewPorts[2]);
    rt.setView(miniView);
    for (const auto& chunk : m_activeChunks)
    {
        rt.draw(*chunk, states);
    }

    rt.setView(oldView);
#endif //_DEBUG_
}

void TerrainComponent::registerWindow()
{
    xy::App::addUserWindow(
        [this]()
    {
        nim::Begin("Info");
        std::string pp("Player Position: " + std::to_string(m_playerPosition.x) + ", " + std::to_string(m_playerPosition.y));
        nim::Text(pp.c_str());
        if (m_currentChunk)
        {
            pp = "ChunkID: " + std::to_string(m_currentChunk->getID());
            nim::Text(pp.c_str());
            pp = "Chunk Position: " + std::to_string(m_currentChunk->getPosition().x) + ", " + std::to_string(m_currentChunk->getPosition().y);
            nim::Text(pp.c_str());
        }
        pp = "Active chunk count: " + std::to_string(m_activeChunks.size());
        nim::Text(pp.c_str());
        nim::End();
    }, this);
}