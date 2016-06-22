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

#include <UserInterface.hpp>

#include <xygine/App.hpp>
#include <xygine/Config.hpp>
#include <xygine/imgui/imgui.h>

namespace
{
    struct Item final
    {
        std::function<void()> draw;
        const void* owner = nullptr;
    };

    std::vector<Item> items;
}

UserInterface::UserInterface()
{
    xy::App::addUserWindow([this]()
    {
        nim::SetNextWindowSize({ 400.f, 400.f });
        nim::Begin("Physics Sandbox");



        //draw added items in sub window
        for (const auto& item : items) item.draw();


        nim::End();
    });
}

//public
void UserInterface::addItem(const std::function<void()>& item, const void* owner)
{
    items.emplace_back();
    items.back().draw = item;
    items.back().owner = owner;
}

void UserInterface::removeItem(const void* owner)
{
    XY_ASSERT(owner, "don't do this!");

    items.erase(std::remove_if(items.begin(), items.end(), 
        [owner](const Item& item)
    {
        return item.owner == owner;
    }), items.end());
}