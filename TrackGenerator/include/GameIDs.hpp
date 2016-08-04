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

#ifndef XYR_GAME_IDS_HPP_
#define XYR_GAME_IDS_HPP_

enum StateID
{
    Racing = 0
};

enum ShaderID
{
    ColouredSmooth = 0,
    ColouredBumped,
    TexturedSmooth,
    TexturedBumped,
    ShadowCaster
};

enum MaterialID
{
    Track = 0,
    Barrier,
    Thing
};

enum ModelID
{
    Cube,
    Quad
};

enum PhysCat
{
    SmallBody = 0x1,
    Trap = 0x2,
    Wall = 0x4,
    Destroyer = 0x8
};

#endif //XYR_GAME_IDS_HPP_