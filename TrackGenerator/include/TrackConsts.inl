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

#ifndef XYR_TRACK_CONSTS_INL_
#define XYR_TRACK_CONSTS_INL_

namespace
{
    const float sectionSize = 1650.f;// xy::DefaultSceneSize.y;
    const float connectionWidth = 520.f;
    const float connectionHeight = sectionSize / 4.f;
    const float connectionBend = 300.f;
    const float connectionGap = (sectionSize - (connectionWidth * 3.f)) / 2.f;
}
#endif //XYR_TRACK_CONSTS_INL_
