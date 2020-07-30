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

#include "DeviceResources.h"

#include <yave/graphics/shaders/SpirVData.h>
#include <yave/graphics/shaders/ShaderModule.h>
#include <yave/graphics/shaders/ComputeProgram.h>
#include <yave/material/Material.h>
#include <yave/meshes/MeshData.h>
#include <yave/meshes/StaticMesh.h>
#include <yave/graphics/images/IBLProbe.h>

#include <y/core/Chrono.h>
#include <y/io2/File.h>
#include <y/utils/log.h>
#include <y/utils/format.h>

namespace yave {

using SpirV = DeviceResources::SpirV;
using ComputePrograms = DeviceResources::ComputePrograms;
using MaterialTemplates = DeviceResources::MaterialTemplates;
using Textures = DeviceResources::Textures;

struct DeviceMaterialData {
	const SpirV frag;
	const SpirV vert;
	const DepthTestMode depth_test;
	const BlendMode blend_mode;
	const CullMode cull_mode;

	static constexpr DeviceMaterialData screen(SpirV frag, bool blended = false) {
		return DeviceMaterialData{frag, SpirV::ScreenVert, DepthTestMode::None, blended ? BlendMode::Add : BlendMode::None, CullMode::None};
	}

	static constexpr DeviceMaterialData basic(SpirV frag) {
		return DeviceMaterialData{frag, SpirV::BasicVert, DepthTestMode::Standard, BlendMode::None,  CullMode::Back};
	}

	static constexpr DeviceMaterialData skinned(SpirV frag) {
		return DeviceMaterialData{frag, SpirV::SkinnedVert, DepthTestMode::Standard, BlendMode::None, CullMode::Back};
	}
};

static constexpr DeviceMaterialData material_datas[] = {
		DeviceMaterialData::basic(SpirV::TexturedFrag),
		DeviceMaterialData::basic(SpirV::TexturedAlphaFrag),
		DeviceMaterialData::skinned(SpirV::TexturedFrag),
		DeviceMaterialData::screen(SpirV::ToneMapFrag),
		DeviceMaterialData::screen(SpirV::PassthroughFrag),
		DeviceMaterialData::screen(SpirV::PassthroughFrag, true),
		DeviceMaterialData::screen(SpirV::BloomFrag, true),
		DeviceMaterialData::screen(SpirV::HBlurFrag, true),
		DeviceMaterialData::screen(SpirV::VBlurFrag, true),
	};

static constexpr const char* spirv_names[] = {
		"equirec_convolution.comp",
		"cubemap_convolution.comp",
		"brdf_integrator.comp",
		"deferred_ambient.comp",
		"deferred_locals.comp",
		"ssao.comp",
		"copy.comp",
		"histogram_clear.comp",
		"histogram.comp",
		"tonemap_params.comp",
		"depth_bounds.comp",

		"tonemap.frag",
		"textured.frag",
		"textured_alpha.frag",
		"passthrough.frag",
		"bloom.frag",
		"hblur.frag",
		"vblur.frag",

		"basic.vert",
		"skinned.vert",
		"screen.vert",
	};

// ABGR
static constexpr std::array<u32, 6> texture_colors[] = {
		{0, 0, 0, 0},
		{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},
		{0xFF7F7F7F, 0xFF7F7F7F, 0xFF7F7F7F, 0xFF7F7F7F},
		{0xFF0000FF, 0xFF0000FF, 0xFF0000FF, 0xFF0000FF},
		{0x00FF7F7F, 0x00FF7F7F, 0x00FF7F7F, 0x00FF7F7F},
		{0xFFEBCE87, 0xFFEBCE87, 0xFF272E3A, 0xFF272E3A}
	};

static constexpr usize spirv_count = usize(SpirV::MaxSpirV);
static constexpr usize compute_count = usize(ComputePrograms::MaxComputePrograms);
static constexpr usize template_count = usize(MaterialTemplates::MaxMaterialTemplates);
static constexpr usize texture_count = usize(Textures::MaxTextures);

static_assert(sizeof(spirv_names) / sizeof(spirv_names[0]) == spirv_count);
static_assert(sizeof(material_datas) / sizeof(material_datas[0]) == template_count);
static_assert(sizeof(texture_colors) / sizeof(texture_colors[0]) == texture_count);

// implemented in DeviceResourcesData.cpp
MeshData cube_mesh_data();
MeshData sphere_mesh_data();
MeshData simple_sphere_mesh_data();
MeshData sweep_mesh_data();


static Texture create_brdf_lut(DevicePtr dptr, const ComputeProgram& brdf_integrator, usize size = 512) {
	y_profile();
	core::DebugTimer _("create_ibl_lut()");

	StorageTexture image(dptr, ImageFormat(VK_FORMAT_R16G16_UNORM), {size, size});

	const DescriptorSet dset(dptr, {Descriptor(StorageView(image))});

	CmdBufferRecorder recorder = dptr->create_disposable_cmd_buffer();
	{
		const auto region = recorder.region("create_brdf_lut");
		recorder.dispatch_size(brdf_integrator, image.size(), {dset});
	}
	dptr->graphic_queue().submit<SyncSubmit>(RecordedCmdBuffer(std::move(recorder)));

	return image;
}



DeviceResources::DeviceResources() {
}

DeviceResources::DeviceResources(DevicePtr dptr) {
	init(dptr);
}

void DeviceResources::init(DevicePtr dptr) {
	y_always_assert(!is_init(), "DeviceResources has already been initialized");

	_device = dptr;

#ifdef Y_DEBUG
	_lock = std::make_unique<std::recursive_mutex>();
#endif

	_spirv = std::make_unique<SpirVData[]>(spirv_count);
	_computes = std::make_unique<ComputeProgram[]>(compute_count);
	_material_templates = std::make_unique<MaterialTemplate[]>(template_count);
	_textures = std::make_unique<AssetPtr<Texture>[]>(texture_count);

	// Load textures here because they won't change and might get packed into descriptor sets that won't be reloaded (in the case of material defaults)
	for(usize i = 0; i != texture_count; ++i) {
		const u8* data = reinterpret_cast<const u8*>(texture_colors[i].data());
		_textures[i] = make_asset<Texture>(dptr, ImageData(math::Vec2ui(2), data, VK_FORMAT_R8G8B8A8_UNORM));
	}

	load_resources();
}

DeviceResources::DeviceResources(DeviceResources&& other) {
	swap(other);
}

DeviceResources& DeviceResources::operator=(DeviceResources&& other) {
	swap(other);
	return *this;
}

DeviceResources::~DeviceResources() {
}

bool DeviceResources::is_init() const {
	return _device;
}

void DeviceResources::swap(DeviceResources& other) {
	std::swap(_device, other._device);

#ifdef Y_DEBUG
	const auto lock = y_profile_unique_lock(*_lock);
	std::unique_lock<std::recursive_mutex> other_lock;
	if(other._lock) {
		other_lock = y_profile_unique_lock(*other._lock);
	}

	std::swap(other._lock, _lock);
	std::swap(other._programs, _programs);
#endif

	std::swap(other._spirv, _spirv);
	std::swap(other._computes, _computes);
	std::swap(other._material_templates, _material_templates);
	std::swap(other._textures, _textures);
	std::swap(other._materials, _materials);
	std::swap(other._meshes, _meshes);
	std::swap(other._probe, _probe);
	std::swap(other._empty_probe, _empty_probe);
	std::swap(other._brdf_lut, _brdf_lut);
}

void DeviceResources::load_resources() {
	y_always_assert(is_init(), "DeviceResources has not been initialized");

	for(usize i = 0; i != spirv_count; ++i) {
		const auto file_name = fmt("%.spv", spirv_names[i]);
		_spirv[i] = SpirVData::deserialized(io2::File::open(file_name).expected(fmt_c_str("Unable to open SPIR-V file (%).", file_name)));
	}

	for(usize i = 0; i != compute_count; ++i) {
		_computes[i] = ComputeProgram(ComputeShader(_spirv[i]));
	}

	for(usize i = 0; i != compute_count; ++i) {
		_computes[i] = ComputeProgram(ComputeShader(_spirv[i]));
	}

	for(usize i = 0; i != template_count; ++i) {
		const auto& data = material_datas[i];
		auto template_data = MaterialTemplateData()
				.set_frag_data(_spirv[data.frag])
				.set_vert_data(_spirv[data.vert])
				.set_depth_mode(data.depth_test)
				.set_cull_mode(data.cull_mode)
				.set_blend_mode(data.blend_mode)
			;
		_material_templates[i] = MaterialTemplate(std::move(template_data));
	}

	{
		_materials = std::make_unique<AssetPtr<Material>[]>(usize(MaxMaterials));
		_materials[0] = make_asset<Material>(&_material_templates[usize(TexturedMaterialTemplate)]);
	}

	{
		_meshes = std::make_unique<AssetPtr<StaticMesh>[]>(usize(MaxMeshes));
		_meshes[0] = make_asset<StaticMesh>(device(), cube_mesh_data());
		_meshes[1] = make_asset<StaticMesh>(device(), sphere_mesh_data());
		_meshes[2] = make_asset<StaticMesh>(device(), simple_sphere_mesh_data());
		_meshes[3] = make_asset<StaticMesh>(device(), sweep_mesh_data());
	}

	_brdf_lut = create_brdf_lut(device(), operator[](BRDFIntegratorProgram));
	_probe = make_asset<IBLProbe>(IBLProbe::from_equirec(*operator[](SkyIBLTexture)));
	_empty_probe = make_asset<IBLProbe>(IBLProbe::from_equirec(*operator[](BlackTexture)));
}


DevicePtr DeviceResources::device() const {
	y_debug_assert(is_init());
	return _device;
}

TextureView DeviceResources::brdf_lut() const {
	y_debug_assert(is_init());
	return _brdf_lut;
}

const AssetPtr<IBLProbe>& DeviceResources::ibl_probe() const {
	y_debug_assert(is_init());
	return _probe;
}

const AssetPtr<IBLProbe>& DeviceResources::empty_probe() const {
	y_debug_assert(is_init());
	return _empty_probe;
}

const SpirVData& DeviceResources::operator[](SpirV i) const {
	y_debug_assert(is_init());
	y_debug_assert(usize(i) < usize(MaxSpirV));
	return _spirv[usize(i)];
}

const ComputeProgram& DeviceResources::operator[](ComputePrograms i) const {
	y_debug_assert(is_init());
	y_debug_assert(usize(i) < usize(MaxComputePrograms));
	return _computes[usize(i)];
}

const MaterialTemplate* DeviceResources::operator[](MaterialTemplates i) const {
	y_debug_assert(is_init());
	y_debug_assert(usize(i) < usize(MaxMaterialTemplates));
	return &_material_templates[usize(i)];
}

const AssetPtr<Texture>& DeviceResources::operator[](Textures i) const {
	y_debug_assert(is_init());
	y_debug_assert(usize(i) < usize(MaxTextures));
	return _textures[usize(i)];
}

const AssetPtr<Material>& DeviceResources::operator[](Materials i) const {
	y_debug_assert(is_init());
	y_debug_assert(usize(i) < usize(MaxMaterials));
	return _materials[usize(i)];
}

const AssetPtr<StaticMesh>& DeviceResources::operator[](Meshes i) const {
	y_debug_assert(is_init());
	y_debug_assert(usize(i) < usize(MaxMeshes));
	return _meshes[usize(i)];
}


#ifdef Y_DEBUG
const ComputeProgram& DeviceResources::program_from_file(std::string_view file) const {
	y_debug_assert(is_init());
	const auto lock = y_profile_unique_lock(*_lock);
	auto& prog = _programs[file];
	if(!prog) {
		auto spirv = SpirVData::deserialized(io2::File::open(fmt("%.spv", file)).expected("Unable to open SPIR-V file"));
		prog = std::make_unique<ComputeProgram>(ComputeShader( spirv));
	}
	return *prog;
}
#endif

void DeviceResources::reload() {
	y_debug_assert(is_init());
	y_profile();
	main_device()->wait_all_queues();

#ifdef Y_DEBUG
	const auto lock = y_profile_unique_lock(*_lock);
	_programs.clear();
#endif

	load_resources();
	log_msg("Resources reloaded");
}

}
