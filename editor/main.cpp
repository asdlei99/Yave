/*******************************
Copyright (c) 2016-2021 Grégoire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/

#include <editor/utils/crashhandler.h>

#include <y/core/Chrono.h>
#include <y/concurrent/concurrent.h>

#include <y/ecs/EntityWorld.h>
#include <yave/ecs/EntityWorld.h>

#include <y/math/random.h>
#include <y/utils/log.h>
#include <y/utils/format.h>

using namespace y;



template<usize I>
struct C {
    float x = I;

    y_reflect(x)
};


const usize entity_count = 1000;

template<typename W>
void add(W& world) {
    log_msg(ct_type_name<W>());
#define MAYBE_ADD(num) if(rng() & 0x1) { world.template add_components<C<num>>(id); }
    core::DebugTimer _("Create many components");
    math::FastRandom rng;
    for(usize i = 0; i != entity_count; ++i) {
        const auto id = world.create_entity();
        MAYBE_ADD(0)
        MAYBE_ADD(1)
        MAYBE_ADD(2)
        MAYBE_ADD(3)
        MAYBE_ADD(4)
        MAYBE_ADD(5)
        MAYBE_ADD(6)
        MAYBE_ADD(7)
        MAYBE_ADD(8)
        MAYBE_ADD(9)
        MAYBE_ADD(10)
        MAYBE_ADD(11)
        MAYBE_ADD(12)
        MAYBE_ADD(13)
        MAYBE_ADD(14)
        MAYBE_ADD(15)
        MAYBE_ADD(16)
        MAYBE_ADD(17)
        MAYBE_ADD(18)
        MAYBE_ADD(19)
    }
#undef MAYBE_ADD
}

template<typename V>
float iter(V&& view) {
    float sum = 0.0;
    core::DebugTimer _("Iterate 2 components");
    for(const auto& [a, b] : view) {
        sum -= a.x;
        sum += b.x;
    }
    for(usize i = 0; i != 100; ++i) {
        for(const auto& [a, b] : view) {
            sum -= a.x / (b.x + 1.0f);
        }
        sum += sum * 4.0f;
    }

    return sum;
}

int main() {
    {
        y::ecs::EntityWorld world;
        add(world);
        const float res = iter(world.view<C<7>, C<19>>());
        log_msg(fmt("result = %", res));
    }

    {
        yave::ecs::EntityWorld world;
        add(world);
        const float res = iter(world.view<C<7>, C<19>>().components());
        log_msg(fmt("result = %", res));
    }

}

