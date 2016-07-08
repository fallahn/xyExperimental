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

        std::function<void(const std::vector<std::int16_t>&)> addIndices = [this](const std::vector<std::int16_t>& idxArray)
        {
            m_indexArrays.push_back(idxArray);

            xy::ModelBuilder::SubMeshLayout sml;
            sml.indexFormat = xy::Mesh::IndexFormat::I16;
            sml.type = xy::Mesh::PrimitiveType::Triangles;
            sml.data = m_indexArrays.back().data();
            sml.size = m_indexArrays.back().size();
            addSubMeshLayout(sml);
        };
        
        //top two points of centre part
        sf::Vector2f tl(sectionSize, connectionHeight);
        sf::Vector2f tr(0.f, connectionHeight);

        //top bits
        auto bits = m_id & 0xf;
        XY_ASSERT(bits != 0 && bits != 0x8, "can't have empty segments");
        if (bits & 0x4)
        {
            //left
            std::int16_t subMeshStart = static_cast<std::int16_t>(m_vertexCount);
            
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
                subMeshStart + 2,
                subMeshStart + 1,
                subMeshStart + 0,
                subMeshStart + 3,
                subMeshStart + 1,
                subMeshStart + 2,
                subMeshStart + 4,
                subMeshStart + 1,
                subMeshStart + 3
            });

            if (tl.x > 0.f) tl.x = 0.f;
            if (tr.x < connectionWidth + connectionGap) tr.x = connectionWidth + connectionGap;
        }
        if (bits & 0x2)
        {
            //middle
            std::int16_t subMeshStart = static_cast<std::int16_t>(m_vertexCount);

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
                subMeshStart + 2,
                subMeshStart + 1,
                subMeshStart + 0,
                subMeshStart + 3,
                subMeshStart + 1,
                subMeshStart + 2
            });

            if (tl.x > connectionWidth + connectionGap) tl.x = connectionWidth + connectionGap;
            if (tr.x < (connectionWidth * 2.f) + connectionGap) tr.x = (connectionWidth * 2.f) + connectionGap;
        }
        if (bits & 0x1)
        {
            //right
            std::int16_t subMeshStart = static_cast<std::int16_t>(m_vertexCount);

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
                subMeshStart + 3,
                subMeshStart + 1,
                subMeshStart + 2,
                subMeshStart + 4,
                subMeshStart + 1,
                subMeshStart + 3,
                subMeshStart + 4,
                subMeshStart + 0,
                subMeshStart + 1
            });

            if (tl.x >(connectionWidth * 2.f) + connectionGap) tl.x = (connectionWidth * 2.f) + connectionGap;
            if (tr.x < sectionSize) tr.x = sectionSize;
        }

        //bottom two points of centre part
        sf::Vector2f bl(sectionSize, connectionHeight * 3.f);
        sf::Vector2f br(0.f, connectionHeight * 3.f);

        //bottom bits
        bits = (m_id & 0xf0) >> 4;
        XY_ASSERT(bits != 0 && bits != 0x8, "can't have empty segments");
        if (bits & 0x4)
        {
            //left
            std::int16_t subMeshStart = static_cast<std::int16_t>(m_vertexCount);

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
                subMeshStart + 0,
                subMeshStart + 2,
                subMeshStart + 3,
                subMeshStart + 0,
                subMeshStart + 3,
                subMeshStart + 1,
                subMeshStart + 1,
                subMeshStart + 3,
                subMeshStart + 4
            });

            if (bl.x > 0.f) bl.x = 0.f;
            if (br.x < connectionWidth + connectionGap) br.x = connectionWidth + connectionGap;
        }
        if (bits & 0x2)
        {
            //middle
            std::int16_t subMeshStart = static_cast<std::int16_t>(m_vertexCount);

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
                subMeshStart + 0,
                subMeshStart + 2,
                subMeshStart + 1,
                subMeshStart + 1,
                subMeshStart + 2,
                subMeshStart + 3
            });

            if (bl.x > connectionWidth + connectionGap) bl.x = connectionWidth + connectionGap;
            if (br.x < (connectionWidth * 2.f) + connectionGap) br.x = (connectionWidth * 2.f) + connectionGap;
        }
        if (bits & 0x1)
        {
            //right
            std::int16_t subMeshStart = static_cast<std::int16_t>(m_vertexCount);

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
                subMeshStart + 0,
                subMeshStart + 3,
                subMeshStart + 1,
                subMeshStart + 1,
                subMeshStart + 3,
                subMeshStart + 4,
                subMeshStart + 1,
                subMeshStart + 4,
                subMeshStart + 2
            });

            if (bl.x >(connectionWidth * 2.f) + connectionGap) bl.x = (connectionWidth * 2.f) + connectionGap;
            if (br.x < sectionSize) br.x = sectionSize;
        }

        //create the centre part - TODO we already have vertices in place, really we just need to sub mesh here...
        std::int16_t subMeshStart = static_cast<std::int16_t>(m_vertexCount);
        addVertex(tl.x, tl.y, trackFarZ);
        addVertex(bl.x, bl.y, trackFarZ);
        addVertex(tr.x, tr.y, trackFarZ);
        addVertex(br.x, br.y, trackFarZ);

        addIndices({ subMeshStart, subMeshStart + 2, subMeshStart + 1, subMeshStart + 2, subMeshStart + 3, subMeshStart + 1 });
    }
}

xy::VertexLayout TrackMeshBuilder::getVertexLayout() const
{
    xy::VertexLayout vl({ 
        xy::VertexLayout::Element(xy::VertexLayout::Element::Type::Position, 3),
        xy::VertexLayout::Element(xy::VertexLayout::Element::Type::Normal, 3) });
    return vl;
}