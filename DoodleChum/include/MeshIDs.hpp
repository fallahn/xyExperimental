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

#ifndef DC_MESH_IDS_HPP_
#define DC_MESH_IDS_HPP_

#include <xygine/shaders/Default.hpp>

namespace Mesh
{
    enum ID
    {
        Haus = 0,
        Background,
        Bud,
        Furniture,
        MoreFurniture,
        ThirdFurniture,
        Tree,
        Lights,
        Walls,
        Cat,
        Clock,
        Vacuum
    };
}

namespace Material
{
    enum ID
    {
        Haus = 0,
        Background,
        Bud,
        Furniture,
        MoreFurniture,
        ThirdFurniture,
        Lights,
        Walls,
        Cat,
        Clock,
        Vaccum
    };
}

#endif //DC_MESH_IDS_HPP_