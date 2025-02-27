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
#ifndef YAVE_SYSTEMS_RENDERERSYSTEM_H
#define YAVE_SYSTEMS_RENDERERSYSTEM_H

#include <yave/ecs/System.h>

#include <yave/graphics/buffers/Buffer.h>
#include <yave/graphics/buffers/buffers.h>
#include <yave/ecs/SparseComponentSet.h>

#include <yave/framegraph/FrameGraphResourceId.h>
#include <yave/graphics/descriptors/uniforms.h>

#include <y/concurrent/Signal.h>
#include <y/core/Vector.h>


namespace yave {

class RendererSystem : public ecs::System {
    public:
        class TransformManager : NonMovable {
            public:
                static constexpr BufferUsage buffer_usage = BufferUsage::StorageBit | BufferUsage::TransferDstBit | BufferUsage::TransferSrcBit;
                using TransformBuffer = TypedBuffer<uniform::TransformableData, buffer_usage>;


                TypedSubBuffer<uniform::TransformableData, BufferUsage::StorageBit> transform_buffer() const {
                    return _transform_buffer;
                }

                TransformManager(ecs::EntityWorld& world);
                ~TransformManager();

                void tick(bool only_recent);

            private:
                TransformBuffer alloc_buffer(usize size);
                void free_index(const TransformableComponent& tr);
                bool alloc_index(const TransformableComponent& tr);

                TransformBuffer _transform_buffer;
                core::Vector<u32> _free;

                ecs::SparseIdSet _moved;
                ecs::SparseIdSet _prev_moved;

                concurrent::Subscription _transform_destroyed;

                ecs::EntityWorld& _world;
        };




        RendererSystem();

        auto transform_buffer() const  {
            return _transform_manager->transform_buffer();
        }

        void destroy() override;
        void setup() override;
        void tick() override;

    private:
        std::unique_ptr<TransformManager> _transform_manager;

};

}

#endif // YAVE_SYSTEMS_RENDERERSYSTEM_H

