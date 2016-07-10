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

#include <TrackMeshBuilder.hpp>

#include <xygine/Assert.hpp>

#include <SFML/System/Vector3.hpp>

#include <glm/glm.hpp>

#include <array>

namespace
{
    const float trackFarZ = -1.f;
    const float trackNearZ = 80.f;

    const float sectionSize = xy::DefaultSceneSize.y; //ugh, duplicate, at least it's based on a const value....
    const float connectionWidth = 340.f;
    const float connectionHeight = sectionSize / 4.f;
    const float connectionBend = 200.f;
    const float connectionGap = (sectionSize - (connectionWidth * 3.f)) / 2.f;

    const std::size_t reservedIndexArrays = 26u;
}

TrackMeshBuilder::TrackMeshBuilder(sf::Uint8 id, const PointData& pointData)
    : m_id          (id),
    m_pointData     (pointData),
    m_vertexCount   (0),
    m_boundingBox   ({}, { xy::DefaultSceneSize.y, xy::DefaultSceneSize.y, 100.f })
{
    m_indexArrays.reserve(reservedIndexArrays);
}

//public
void TrackMeshBuilder::build()
{
    //remember to update vertex count!
    if (m_vertexData.empty())
    {
        std::function<void(const glm::vec3&, const glm::vec3&)> addVertex = 
            [this](const glm::vec3& position, const glm::vec3& normal)
        {
            m_vertexData.push_back(position.x);
            m_vertexData.push_back(position.y);
            m_vertexData.push_back(position.z);

            //normal
            m_vertexData.push_back(normal.x);
            m_vertexData.push_back(normal.y);
            m_vertexData.push_back(normal.z);

            //TODO tangents


            //TODO UVs


            m_vertexCount++;
        };

        std::function<void(const std::vector<std::uint8_t>&)> addIndices = [this](const std::vector<std::uint8_t>& idxArray)
        {
            m_indexArrays.push_back(idxArray);

            XY_WARNING(m_indexArrays.size() > reservedIndexArrays, "Consider reserving greater index array size: Index array count " + std::to_string(m_indexArrays.size()));

            xy::ModelBuilder::SubMeshLayout sml;
            sml.indexFormat = xy::Mesh::IndexFormat::I8;
            sml.type = xy::Mesh::PrimitiveType::Triangles;
            sml.data = m_indexArrays.back().data();
            sml.size = m_indexArrays.back().size();
            addSubMeshLayout(sml);
        };
        
        //top two points of centre part
        float leftPoint = sectionSize;
        float rightPoint = 0.f;
        std::uint8_t tlIdx, trIdx;

        //top bits
        auto bits = m_id & 0xf;
        XY_ASSERT(bits != 0 && bits != 0x8, "can't have empty segments");
        if (bits & 0x4)
        {
            //left
            auto subMeshStart = m_vertexCount;
            
            //add the vertices
            for (const auto& p : m_pointData[0].first)
            {
                addVertex({ p.x, p.y, trackFarZ }, { 0.f, 0.f, 1.f });
            }

            for (const auto& p : m_pointData[0].second)
            {
                addVertex({ p.x, p.y, trackFarZ }, { 0.f, 0.f, 1.f });
            }

            //create a submesh for this part
            //I wish there was a better way to do this than work it out by hand :/
            addIndices(
            {
                std::uint8_t(subMeshStart + 2),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 0),
                std::uint8_t(subMeshStart + 3),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 2),
                std::uint8_t(subMeshStart + 4),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 3)
            });

            //check if we are the last or first connection
            //and update the edge for the centre section
            if (leftPoint > 0.f)
            {
                leftPoint = 0.f;
                tlIdx = std::uint8_t(subMeshStart + 1);
            }
            if (rightPoint < connectionWidth + connectionGap)
            {
                rightPoint = connectionWidth + connectionGap;
                trIdx = std::uint8_t(subMeshStart + 4);
            }
        }
        if (bits & 0x2)
        {
            //middle
            auto subMeshStart = m_vertexCount;

            //TODO we could potentially optimise this by checking
            //for the previous connection and sharing a vertex

            //add the vertices
            for (const auto& p : m_pointData[1].first)
            {
                addVertex({ p.x, p.y, trackFarZ }, { 0.f, 0.f, 1.f });
            }

            for (const auto& p : m_pointData[1].second)
            {
                addVertex({ p.x, p.y, trackFarZ }, { 0.f, 0.f, 1.f });
            }

            addIndices({
                std::uint8_t(subMeshStart + 2),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 0),
                std::uint8_t(subMeshStart + 3),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 2)
            });

            if (leftPoint > connectionWidth + connectionGap)
            {
                leftPoint = connectionWidth + connectionGap;
                tlIdx = std::uint8_t(subMeshStart + 1);
            }
            if (rightPoint < (connectionWidth * 2.f) + connectionGap)
            {
                rightPoint = (connectionWidth * 2.f) + connectionGap;
                trIdx = std::uint8_t(subMeshStart + 3);
            }
        }
        if (bits & 0x1)
        {
            //right
            auto subMeshStart = m_vertexCount;

            //TODO check for middle section and share a vertex

            //add the vertices
            for (const auto& p : m_pointData[2].first)
            {
                addVertex({ p.x, p.y, trackFarZ }, { 0.f, 0.f, 1.f });
            }

            for (const auto& p : m_pointData[2].second)
            {
                addVertex({ p.x, p.y, trackFarZ }, { 0.f, 0.f, 1.f });
            }

            addIndices({
                std::uint8_t(subMeshStart + 3),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 2),
                std::uint8_t(subMeshStart + 4),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 3),
                std::uint8_t(subMeshStart + 4),
                std::uint8_t(subMeshStart + 0),
                std::uint8_t(subMeshStart + 1)
            });

            if (leftPoint > (connectionWidth * 2.f) + connectionGap)
            {
                leftPoint = (connectionWidth * 2.f) + connectionGap;
                tlIdx = std::uint8_t(subMeshStart);
            }
            if (rightPoint < sectionSize)
            {
                rightPoint = sectionSize;
                trIdx = std::uint8_t(subMeshStart + 4);
            }
        }

        //bottom two points of centre part
        leftPoint = sectionSize;
        rightPoint = 0.f;
        std::uint8_t blIdx, brIdx;

        //bottom bits
        bits = (m_id & 0xf0) >> 4;
        XY_ASSERT(bits != 0 && bits != 0x8, "can't have empty segments");
        if (bits & 0x4)
        {
            //left
            auto subMeshStart = m_vertexCount;

            //add the vertices
            for (const auto& p : m_pointData[3].first)
            {
                addVertex({ p.x, p.y, trackFarZ }, { 0.f, 0.f, 1.f });
            }

            for (const auto& p : m_pointData[3].second)
            {
                addVertex({ p.x, p.y, trackFarZ }, { 0.f, 0.f, 1.f });
            }

            addIndices({
                std::uint8_t(subMeshStart + 0),
                std::uint8_t(subMeshStart + 2),
                std::uint8_t(subMeshStart + 3),
                std::uint8_t(subMeshStart + 0),
                std::uint8_t(subMeshStart + 3),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 3),
                std::uint8_t(subMeshStart + 4)
            });

            if (leftPoint > 0.f)
            {
                leftPoint = 0.f;
                blIdx = std::uint8_t(subMeshStart);
            }
            if (rightPoint < connectionWidth + connectionGap)
            {
                rightPoint = connectionWidth + connectionGap;
                brIdx = std::uint8_t(subMeshStart + 2);
            }
        }
        if (bits & 0x2)
        {
            //middle
            auto subMeshStart = m_vertexCount;

            //add the vertices
            for (const auto& p : m_pointData[4].first)
            {
                addVertex({ p.x, p.y, trackFarZ }, { 0.f, 0.f, 1.f });
            }

            for (const auto& p : m_pointData[4].second)
            {
                addVertex({ p.x, p.y, trackFarZ }, { 0.f, 0.f, 1.f });
            }

            addIndices({
                std::uint8_t(subMeshStart + 0),
                std::uint8_t(subMeshStart + 2),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 2),
                std::uint8_t(subMeshStart + 3)
            });

            if (leftPoint > connectionWidth + connectionGap)
            {
                leftPoint = connectionWidth + connectionGap;
                blIdx = std::uint8_t(subMeshStart);
            }
            if (rightPoint < (connectionWidth * 2.f) + connectionGap)
            {
                rightPoint = (connectionWidth * 2.f) + connectionGap;
                brIdx = std::uint8_t(subMeshStart + 2);
            }
        }
        if (bits & 0x1)
        {
            //right
            auto subMeshStart = m_vertexCount;

            //add the vertices
            for (const auto& p : m_pointData[5].first)
            {
                addVertex({ p.x, p.y, trackFarZ }, { 0.f, 0.f, 1.f });
            }

            for (const auto& p : m_pointData[5].second)
            {
                addVertex({ p.x, p.y, trackFarZ }, { 0.f, 0.f, 1.f });
            }

            addIndices({
                std::uint8_t(subMeshStart + 0),
                std::uint8_t(subMeshStart + 3),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 3),
                std::uint8_t(subMeshStart + 4),
                std::uint8_t(subMeshStart + 1),
                std::uint8_t(subMeshStart + 4),
                std::uint8_t(subMeshStart + 2)
            });

            if (leftPoint > (connectionWidth * 2.f) + connectionGap)
            {
                leftPoint = (connectionWidth * 2.f) + connectionGap;
                blIdx = std::uint8_t(subMeshStart);
            }
            if (rightPoint < sectionSize)
            {
                rightPoint = sectionSize;
                brIdx = std::uint8_t(subMeshStart + 3);
            }
        }

        //create the centre part from existing verts
        addIndices({
            tlIdx,
            trIdx,
            blIdx,
            trIdx,
            brIdx,
            blIdx });

        buildBarriers(addVertex, addIndices);
    }

    //warning if too many vertices because this will break 8 bit indexing
    XY_WARNING(m_vertexCount > 255, "LARGE VERTEX COUNT! Index data likely to be incorrect (more than 255 vertices!");
}

xy::VertexLayout TrackMeshBuilder::getVertexLayout() const
{
    xy::VertexLayout vl({ 
        xy::VertexLayout::Element(xy::VertexLayout::Element::Type::Position, 3),
        xy::VertexLayout::Element(xy::VertexLayout::Element::Type::Normal, 3) });
    return vl;
}

//private
void TrackMeshBuilder::buildBarriers(std::function<void(const glm::vec3&, const glm::vec3&)>& addVertex, std::function<void(const std::vector<std::uint8_t>&)>& addIndices)
{
    std::function<void(const std::array<glm::vec3, 4u>&)> addWall = 
        [this, &addVertex, &addIndices](const std::array<glm::vec3, 4u>& quad)
    {
        //use the cross product of 2 edges to discover the face normal
        glm::vec3 tan = glm::normalize(quad[1] - quad[0]);
        glm::vec3 bitan = glm::normalize(quad[2] - quad[0]);
        glm::vec3 normal = glm::cross(tan, bitan);

        auto firstIndex = m_vertexCount;
        for (const auto& v : quad)
        {
            addVertex(v, normal);
        }
        //TODO log the first submesh ID of the first barrier so we can use  a different material if we like
        addIndices({
            std::uint8_t(firstIndex + 2),
            std::uint8_t(firstIndex + 3),
            std::uint8_t(firstIndex),
            std::uint8_t(firstIndex + 3),
            std::uint8_t(firstIndex + 1),
            std::uint8_t(firstIndex)
        });
    };
    
    auto bits = m_id & 0xf;
    //top
    if (bits & 0x4)
    {
        //create a quad for each  edge.
        //verts are duplicated to stop smoothing on bends
        //verts arranged so that 0-1 will be bottom x axis
        //making sure normal points in correct direction
        std::array<glm::vec3, 4u> quad = 
        {
            glm::vec3(m_pointData[0].first[1].x, m_pointData[0].first[1].y, trackFarZ),
            glm::vec3(m_pointData[0].first[0].x, m_pointData[0].first[0].y, trackFarZ),
            glm::vec3(m_pointData[0].first[1].x, m_pointData[0].first[1].y, trackNearZ),
            glm::vec3(m_pointData[0].first[0].x, m_pointData[0].first[0].y, trackNearZ)
        };
        addWall(quad);
 
        //2 quads make up right edge - TODO we can cull these if capping hides them
        quad =
        {
            glm::vec3(m_pointData[0].second[1].x, m_pointData[0].second[1].y, trackFarZ),
            glm::vec3(m_pointData[0].second[0].x, m_pointData[0].second[0].y, trackFarZ),
            glm::vec3(m_pointData[0].second[1].x, m_pointData[0].second[1].y, trackNearZ),
            glm::vec3(m_pointData[0].second[0].x, m_pointData[0].second[0].y, trackNearZ)
        };
        addWall(quad);

        quad =
        {
            glm::vec3(m_pointData[0].second[2].x, m_pointData[0].second[2].y, trackFarZ),
            glm::vec3(m_pointData[0].second[1].x, m_pointData[0].second[1].y, trackFarZ),
            glm::vec3(m_pointData[0].second[2].x, m_pointData[0].second[2].y, trackNearZ),
            glm::vec3(m_pointData[0].second[1].x, m_pointData[0].second[1].y, trackNearZ)
        };
        addWall(quad);
    }

    if (bits & 0x2)
    {
        std::array<glm::vec3, 4u> quad = 
        {
            glm::vec3(m_pointData[1].first[1].x, m_pointData[1].first[1].y, trackFarZ),
            glm::vec3(m_pointData[1].first[0].x, m_pointData[1].first[0].y, trackFarZ),
            glm::vec3(m_pointData[1].first[1].x, m_pointData[1].first[1].y, trackNearZ),
            glm::vec3(m_pointData[1].first[0].x, m_pointData[1].first[0].y, trackNearZ)
        };
        addWall(quad);

        quad =
        {
            glm::vec3(m_pointData[1].second[0].x, m_pointData[1].second[0].y, trackFarZ),
            glm::vec3(m_pointData[1].second[1].x, m_pointData[1].second[1].y, trackFarZ),
            glm::vec3(m_pointData[1].second[0].x, m_pointData[1].second[0].y, trackNearZ),
            glm::vec3(m_pointData[1].second[1].x, m_pointData[1].second[1].y, trackNearZ)
        };
        addWall(quad);
    }

    if (bits & 0x1)
    {
        std::array<glm::vec3, 4u> quad =
        {
            glm::vec3(m_pointData[2].second[0].x, m_pointData[2].second[0].y, trackFarZ),
            glm::vec3(m_pointData[2].second[1].x, m_pointData[2].second[1].y, trackFarZ),
            glm::vec3(m_pointData[2].second[0].x, m_pointData[2].second[0].y, trackNearZ),
            glm::vec3(m_pointData[2].second[1].x, m_pointData[2].second[1].y, trackNearZ)
        };
        addWall(quad);

        //TODO decide if we want to bother with inside walls
    }

    bits = (m_id & 0xf0) >> 4;
    //bottom
    if (bits & 0x4)
    {

    }

    if (bits & 0x2)
    {

    }

    if (bits & 0x1)
    {

    }

    //TODO add barriers for centre part
    //TODO barriers for end caps when middle is missing
}