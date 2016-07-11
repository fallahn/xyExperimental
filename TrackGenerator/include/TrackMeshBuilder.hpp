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

#ifndef XYR_TRACKMESH_BUILDER_HPP_
#define XYR_TRACKMESH_BUILDER_HPP_

#include <xygine/mesh/ModelBuilder.hpp>

#include <functional>

class TrackMeshBuilder : public xy::ModelBuilder
{
    using PointData = std::array<std::pair<std::vector<sf::Vector2f>, std::vector<sf::Vector2f>>, 6u>;
public:
    TrackMeshBuilder(sf::Uint8, const PointData&);
    ~TrackMeshBuilder() = default;

    void build() override;
    xy::VertexLayout getVertexLayout() const override;
    const float* getVertexData() const override { return m_vertexData.data(); }
    std::size_t getVertexCount() const override { return m_vertexCount; }
    const xy::BoundingBox& getBoundingBox() const override { return m_boundingBox; }

    std::size_t getFirstBarrierIndex() const { return m_firstBarrierIndex; }
private:
    sf::Uint8 m_id;
    const PointData& m_pointData;

    std::vector<float> m_vertexData;
    std::size_t m_vertexCount;
    xy::BoundingBox m_boundingBox;

    std::vector<std::vector<std::uint8_t>> m_indexArrays;

    std::size_t m_firstBarrierIndex;
    void buildBarriers(std::function<void(const glm::vec3&, const glm::vec3&, const glm::vec3&, const glm::vec3&, const glm::vec2&)>&,
        std::function<void(const std::vector<std::uint8_t>&)>&);
};

#endif // XYR_TRACKMESH_BUILDER_HPP_