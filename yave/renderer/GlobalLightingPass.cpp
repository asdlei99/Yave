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

#include "GlobalLightingPass.h"
#include "LightingPass.h"

#include <yave/device/Device.h>

#include <yave/framegraph/FrameGraph.h>
#include <yave/framegraph/FrameGraphPassBuilder.h>

#include <yave/components/PointLightComponent.h>
#include <yave/components/SpotLightComponent.h>
#include <yave/components/TransformableComponent.h>
#include <yave/ecs/EntityWorld.h>
#include <yave/entities/entities.h>

#include <yave/meshes/AABB.h>

namespace yave {

struct GIVolume {
	//AABB aabb;
	std::unique_ptr<TypedBuffer<uniform::SH, BufferUsage::StorageBit>> sh_volume;

	void create_sh_volume(DevicePtr dptr) {
		sh_volume = std::make_unique<TypedBuffer<uniform::SH, BufferUsage::StorageBit>>(dptr, 1000);
	}

	math::Vec3 probe_position(usize index) const {
		const math::Vec3 uvw = math::Vec3(index / 100, (index / 10) % 10, index % 10) * 0.1f;
		return (uvw - 0.5f) * 2000.0f;
		//return aabb.min() + aabb.extent() * uvw;
	}
};

void update_probe(FrameGraph& framegraph, const ecs::EntityWorld& world, const GIVolume& volume, usize index) {
	static constexpr usize gi_probe_face_size = 8;

	const u32 sh_index = index % volume.sh_volume->size();
	{
		FrameGraphPassBuilder builder = framegraph.add_pass("SH clear pass");
		builder.add_external_input(Descriptor(*volume.sh_volume), 0, PipelineStage::ComputeBit);
		builder.set_render_func([=](CmdBufferRecorder& recorder, const FrameGraphPass* self) {
			const ComputeProgram& program = recorder.device()->device_resources().program_from_file("clear_sh.comp");
			recorder.dispatch(program, math::Vec3ui(1), {self->descriptor_sets()[0]}, sh_index);
		});
	}


	for(usize i = 0; i != 6; ++i) {
		const math::Vec3 position = volume.probe_position(sh_index);

		math::Vec3 axis;
		axis[i % 3] = i > 2 ? -1.0f : 1.0f;
		const math::Vec3 up = i % 3 == 2 ? math::Vec3(1.0f, 0.0f, 0.0f) : math::Vec3(0.0f, 0.0f, 1.0f);

		Camera camera;
		camera.set_view(math::look_at(position, position + axis, up));
		camera.set_proj(math::perspective(math::to_rad(90.0f), 1.0f, 0.1f));

		const SceneView scene(&world, camera);
		const GBufferPass gbuffer = GBufferPass::create(framegraph, scene, math::Vec2ui(gi_probe_face_size));
		const LightingPass lighting = LightingPass::create(framegraph, gbuffer);

		{
			FrameGraphPassBuilder sh_builder = framegraph.add_pass("SH building pass");

			sh_builder.add_uniform_input(gbuffer.depth, 0, PipelineStage::ComputeBit);
			sh_builder.add_uniform_input(lighting.lit, 0, PipelineStage::ComputeBit);
			sh_builder.add_uniform_input(gbuffer.scene_pass.camera_buffer, 0, PipelineStage::ComputeBit);
			sh_builder.add_external_input(Descriptor(*volume.sh_volume), 0, PipelineStage::ComputeBit);
			sh_builder.set_render_func([=](CmdBufferRecorder& recorder, const FrameGraphPass* self) {
				const ComputeProgram& program = recorder.device()->device_resources().program_from_file("build_sh.comp");
				recorder.dispatch_size(program, math::Vec2ui(1), {self->descriptor_sets()[0]}, sh_index);
			});
		}
	}
}


GlobalLightingPass GlobalLightingPass::create(FrameGraph& framegraph, const GBufferPass& gbuffer) {
	static usize index = 0;
	static GIVolume volume;

	if(!volume.sh_volume) {
		volume.create_sh_volume(framegraph.device());
	}

	update_probe(framegraph, gbuffer.scene_pass.scene_view.world(), volume, index++);


	const math::Vec2ui size = framegraph.image_size(gbuffer.depth);
	FrameGraphPassBuilder builder = framegraph.add_pass("GI apply pass");
	const auto lit = builder.declare_image(VK_FORMAT_R16G16B16A16_SFLOAT, size);

	{
		builder.add_uniform_input(gbuffer.depth, 0, PipelineStage::ComputeBit);
		builder.add_uniform_input(gbuffer.color, 0, PipelineStage::ComputeBit);
		builder.add_uniform_input(gbuffer.normal, 0, PipelineStage::ComputeBit);
		builder.add_uniform_input(gbuffer.scene_pass.camera_buffer, 0, PipelineStage::ComputeBit);
		builder.add_external_input(Descriptor(*volume.sh_volume), 0, PipelineStage::ComputeBit);
		builder.add_storage_output(lit, 0, PipelineStage::ComputeBit);
		builder.set_render_func([=](CmdBufferRecorder& recorder, const FrameGraphPass* self) {
			const ComputeProgram& program = recorder.device()->device_resources().program_from_file("sh_lighting.comp");
			recorder.dispatch_size(program, size, {self->descriptor_sets()[0]});
		});
	}

	GlobalLightingPass pass;
	pass.lit = lit;
	return pass;
}


/*

FrameGraphPassBuilder gather_builder = framegraph.add_pass("Surfel gather pass");

const auto surfel_buffer = gather_builder.declare_typed_buffer<uniform::Surfel>(surfel_count);

gather_builder.add_uniform_input(gbuffer.depth, 0, PipelineStage::ComputeBit);
gather_builder.add_uniform_input(gbuffer.color, 0, PipelineStage::ComputeBit);
gather_builder.add_uniform_input(gbuffer.normal, 0, PipelineStage::ComputeBit);
gather_builder.add_uniform_input(gbuffer.scene_pass.camera_buffer, 0, PipelineStage::ComputeBit);
gather_builder.add_storage_output(surfel_buffer, 0, PipelineStage::ComputeBit);
gather_builder.set_render_func([=](CmdBufferRecorder& recorder, const FrameGraphPass* self) {
	const ComputeProgram& program = recorder.device()->device_resources().program_from_file("gather_surfels.comp");
	recorder.dispatch(program, math::Vec3ui(1), {self->descriptor_sets()[0]});
});


FrameGraphPassBuilder light_builder = framegraph.add_pass("Surfel lighting pass");

const auto point_buffer = light_builder.declare_typed_buffer<uniform::PointLight>(max_point_lights);
const auto spot_buffer = light_builder.declare_typed_buffer<uniform::SpotLight>(max_spot_lights);
const auto irradiance_buffer = light_builder.declare_typed_buffer<math::Vec3>(surfel_count);

light_builder.add_storage_input(surfel_buffer, 0, PipelineStage::ComputeBit);
light_builder.add_storage_output(irradiance_buffer, 0, PipelineStage::ComputeBit);
light_builder.add_storage_input(point_buffer, 0, PipelineStage::ComputeBit);
light_builder.add_storage_input(spot_buffer, 0, PipelineStage::ComputeBit);
light_builder.map_update(point_buffer);
light_builder.map_update(spot_buffer);
light_builder.set_render_func([=](CmdBufferRecorder& recorder, const FrameGraphPass* self) {
	struct PushData {
		u32 point_count;
		u32 spot_count;
		u32 surfel_count;
	} push_data {0, 0, surfel_count};

	{
		TypedMapping<uniform::PointLight> mapping = self->resources().mapped_buffer(point_buffer);
		for(auto [t, l] : scene.world().view(PointLightArchetype()).components()) {
			mapping[push_data.point_count++] = {
				t.position(),
				l.radius(),
				l.color() * l.intensity(),
				std::max(math::epsilon<float>, l.falloff())
			};
		}
	}

	{
		TypedMapping<uniform::SpotLight> mapping = self->resources().mapped_buffer(spot_buffer);
		for(auto [t, l] : scene.world().view(SpotLightArchetype()).components()) {
			mapping[push_data.spot_count++] = {
				t.position(),
				l.radius(),
				l.color() * l.intensity(),
				std::max(math::epsilon<float>, l.falloff()),
				t.forward(),
				std::cos(l.half_angle()),
				std::max(math::epsilon<float>, l.angle_exponent()),
				u32(-1),
				{}
			};
		}
	}

	const ComputeProgram& program = recorder.device()->device_resources().program_from_file("light_surfels.comp");
	recorder.dispatch_size(program, math::Vec2ui(surfel_count, 1), {self->descriptor_sets()[0]}, push_data);
});
*/
/*for(usize i = 0; i != 6; ++i) {
	math::Vec3 axis;
	axis[i % 3] = i > 2 ? 1.0f : -1.0f;
	const math::Vec3 up = i == 2
		? math::Vec3(1.0f, 0.0f, 0.0f)
		: math::Vec3(0.0f, 0.0f, 1.0f);


	Camera camera;
	camera.set_view(math::look_at(probe_pos, probe_pos + axis, up));
	camera.set_proj(math::perspective(math::to_rad(45.0f), 1.0f, 0.1f));

	const SceneView face_view(&view.world(), camera);
	const GBufferPass gbuffer = GBufferPass::create(framegraph, face_view, face_size);

	{
		FrameGraphPassBuilder builder = framegraph.add_pass("Surfel gather pass");

		if(!i) {
			surfels = builder.declare_typed_buffer<uniform::Surfel>(surfel_count);
		}

		builder.add_uniform_input(gbuffer.depth, 0, PipelineStage::ComputeBit);
		builder.add_uniform_input(gbuffer.color, 0, PipelineStage::ComputeBit);
		builder.add_uniform_input(gbuffer.normal, 0, PipelineStage::ComputeBit);
		builder.add_uniform_input(gbuffer.scene_pass.camera_buffer, 0, PipelineStage::ComputeBit);
		builder.add_storage_output(surfels, 0, PipelineStage::ComputeBit);
		builder.set_render_func([=](CmdBufferRecorder& recorder, const FrameGraphPass* self) {
			const ComputeProgram& program = recorder.device()->device_resources().program_from_file("surfel_gather.comp");
			recorder.dispatch(program, math::Vec3ui(1), {self->descriptor_sets()[0]});
		});
	}
}*/

}
