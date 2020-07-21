/*******************************
Copyright (c) 2016-2020 Grégoire Angerand

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

#include "SceneRenderSubPass.h"

#include <yave/framegraph/FrameGraph.h>

#include <yave/ecs/EntityWorld.h>

#include <yave/components/TransformableComponent.h>
#include <yave/components/StaticMeshComponent.h>
#include <yave/entities/entities.h>


namespace yave {

SceneRenderSubPass SceneRenderSubPass::create(FrameGraphPassBuilder& builder, const SceneView& view) {
	auto camera_buffer = builder.declare_typed_buffer<Renderable::CameraData>();
	const auto transform_buffer = builder.declare_typed_buffer<math::Transform<>>(max_batch_size);

	SceneRenderSubPass pass;
	pass.scene_view = view;
	pass.descriptor_set_index = builder.next_descriptor_set_index();
	pass.camera_buffer = camera_buffer;
	pass.transform_buffer = transform_buffer;

	builder.add_uniform_input(camera_buffer, pass.descriptor_set_index);
	builder.add_attrib_input(transform_buffer);
	builder.map_update(camera_buffer);
	builder.map_update(transform_buffer);

	return pass;
}



static math::Vec3 probe_position(usize index) {
	const math::Vec3 uvw = math::Vec3(index / 100, (index / 10) % 10, index % 10) * 0.1f;
	return (uvw - 0.5f) * 2000.0f;
	//return aabb.min() + aabb.extent() * uvw;
}

static usize render_world(const SceneRenderSubPass* sub_pass, RenderPassRecorder& recorder, const FrameGraphPass* pass, usize index = 0) {
	y_profile();
	const auto region = recorder.region("Scene");

	const ecs::EntityWorld& world = sub_pass->scene_view.world();

	auto transform_mapping = pass->resources().mapped_buffer(sub_pass->transform_buffer);
	const auto transforms = pass->resources().buffer<BufferUsage::AttributeBit>(sub_pass->transform_buffer);
	const auto& descriptor_set = pass->descriptor_sets()[sub_pass->descriptor_set_index];

	recorder.bind_attrib_buffers({}, {transforms});

	for(const auto& [tr, me] : world.view(StaticMeshArchetype()).components()) {
		transform_mapping[index] = tr.transform();
		me.render(recorder, Renderable::SceneData{descriptor_set, u32(index)});
		++index;
	}


	{
		const auto mat = recorder.device()->device_resources()[DeviceResources::EmptyMaterial];
		const auto mat_templ = mat->material_template();
		recorder.bind_material(mat_templ, {descriptor_set, mat->descriptor_set()});
		const auto mesh = recorder.device()->device_resources()[DeviceResources::SphereMesh];
		recorder.bind_buffers(TriangleSubBuffer(mesh->triangle_buffer()), VertexSubBuffer(mesh->vertex_buffer()));
		VkDrawIndexedIndirectCommand indirect = mesh->indirect_data();
		indirect.firstInstance = index;
		indirect.instanceCount = 1000;
		recorder.draw(indirect);
		for(usize i = 0; i != 1000; ++i) {
			transform_mapping[index] = math::Transform<>(probe_position(i), math::Quaternion<>(), math::Vec3(10.0f));
			++index;
		}
	}

	return index;
}


void SceneRenderSubPass::render(RenderPassRecorder& recorder, const FrameGraphPass* pass) const {
	// fill render data
	auto camera_mapping = pass->resources().mapped_buffer(camera_buffer);
	camera_mapping[0] = scene_view.camera();

	usize index = 0;
	if(scene_view.has_world()) {
		index = render_world(this, recorder, pass, index);
	}
}

}
