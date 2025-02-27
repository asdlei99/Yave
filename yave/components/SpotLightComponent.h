/*******************************
Copyright (c) 2016-2023 Grégoire Angerand

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
#ifndef YAVE_COMPONENTS_SPOTLIGHTCOMPONENT_H
#define YAVE_COMPONENTS_SPOTLIGHTCOMPONENT_H

#include <yave/ecs/ecs.h>

#include <y/reflect/reflect.h>

#include "LocalLightBase.h"
#include "TransformableComponent.h"

namespace yave {

class SpotLightComponent final :
        public LocalLightBase,
        public ecs::SystemLinkedComponent<SpotLightComponent, AABBUpdateSystem> {

    public:
        struct EnclosingSphere {
            float dist_to_center;
            float radius;
        };

        SpotLightComponent() = default;

        EnclosingSphere enclosing_sphere() const;

        float& half_angle();
        float half_angle() const;

        float& half_inner_angle();
        float half_inner_angle() const;

        bool& cast_shadow();
        bool cast_shadow() const;

        u32& shadow_lod();
        u32 shadow_lod() const;

        AABB aabb() const;

        math::Vec2 attenuation_scale_offset() const;

        void inspect(ecs::ComponentInspector* inspector);

        y_reflect(SpotLightComponent,
            _color, _intensity, _range, _min_radius, _falloff,
            _half_angle, _half_inner_angle, _cast_shadow, _shadow_lod
        )

    private:
        float _half_angle = math::to_rad(45.0f);
        float _half_inner_angle = math::to_rad(30.0f);

        bool _cast_shadow = false;
        u32 _shadow_lod = 0;
};

}

#endif // YAVE_COMPONENTS_SPOTLIGHTCOMPONENT_H

