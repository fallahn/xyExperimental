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

#include <array>

namespace
{
    const float trackFarZ = -1.f;

    const float sectionSize = xy::DefaultSceneSize.y; //ugh, duplicate, at least it's based on a const value....
    const float connectionWidth = 340.f;
    const float connectionHeight = sectionSize / 4.f;
    const float connectionBend = 200.f;
    const float connectionGap = (sectionSize - (connectionWidth * 3.f)) / 2.f;
}

TrackMeshBuilder::TrackMeshBuilder(sf::Uint8 id, const PointData& pointData)
    : m_id          (id),
    m_pointData     (pointData),
    m_vertexCount   (0),
    m_boundingBox   ({}, { xy::DefaultSceneSize.y, xy::DefaultSceneSize.y, 100.f })
{
    m_indexArrays.reserve(10); //TODO increase when we add walls / islands
}

//public
void TrackMeshBuilder::build()
{
    //remember to update vertex count!
    if (m_vertexData.empty())
    {
        std::function<void(float, float, float)> addVertex = [this](float x, float y, float z)
        {
            m_vertexData.push_back(x);
            m_vertexData.push_back(y);
            m_vertexData.push_back(z);

            //normal
            m_vertexData.push_back(0.f);
            m_vertexData.push_back(0.f);
            m_vertexData.push_back(1.f);

            //TODO tangents


            //TODO UVs


            m_vertexCount++;
        };

        std::function<void(const std::vector<std::uint8_t>&)> addIndices = [this](const std::vector<std::uint8_t>& idxArray)
        {
            m_indexArrays.push_back(idxArray);

            XY_WARNING(m_indexArrays.size() > 10, "Consider reserving greater index array size: Index array count " + std::to_string(m_indexArrays.size()));

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
                addVertex(p.x, p.y, trackFarZ);
            }

            for (const auto& p : m_pointData[0].second)
            {
                addVertex(p.x, p.y, trackFarZ);
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
                addVertex(p.x, p.y, trackFarZ);
            }

            for (const auto& p : m_pointData[1].second)
            {
                addVertex(p.x, p.y, trackFarZ);
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
                addVertex(p.x, p.y, trackFarZ);
            }

            for (const auto& p : m_pointData[2].second)
            {
                addVertex(p.x, p.y, trackFarZ);
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
                addVertex(p.x, p.y, trackFarZ);
            }

            for (const auto& p : m_pointData[3].second)
            {
                addVertex(p.x, p.y, trackFarZ);
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
                addVertex(p.x, p.y, trackFarZ);
            }

            for (const auto& p : m_pointData[4].second)
            {
                addVertex(p.x, p.y, trackFarZ);
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
                addVertex(p.x, p.y, trackFarZ);
            }

            for (const auto& p : m_pointData[5].second)
            {
                addVertex(p.x, p.y, trackFarZ);
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