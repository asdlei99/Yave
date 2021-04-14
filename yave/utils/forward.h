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
#ifndef YAVE_UTILS_FORWARD_H
#define YAVE_UTILS_FORWARD_H

// Auto generated: forward definitions for most non template classes
// TODO: Nested classes will not be declared correctly

namespace yave {
struct FrameGraphMutableBufferId;
struct SSAOPass;
struct BloomSettings;
struct BoneKey;
class FileSystemModel;
class Mapping;
class ThreadLocalDevice;
class SwapchainImage;
class FrameGraphResourcePool;
class IBLProbe;
class DescriptorSetLayout;
class FrameGraphPassBuilder;
struct Box;
class DescriptorSetAllocator;
class DeviceMemoryView;
struct CmdBuffer;
class DebugParams;
class ShaderModuleBase;
class AssetLoaderSystem;
struct FreeBlock;
class Framebuffer;
class DescriptorSetPool;
class FrameGraphPass;
class BufferBase;
class SkyLightComponent;
struct SkeletonData;
struct LightingSettings;
struct Viewport;
struct Registerer;
struct ResourceCreateInfo;
struct ImageCreateInfo;
struct LayoutPools;
struct Attribute;
struct BloomPass;
struct DownsamplePass;
struct Region;
class GenericAssetPtr;
class DirectionalLightComponent;
class LocalFileSystemModel;
struct SubMesh;
struct InlineBlock;
struct Semaphores;
struct ShadowMapPass;
class DeviceMemoryHeapBase;
class ImageBarrier;
struct AssetData;
class DebugUtils;
struct FrameGraphImageId;
class OctreeEntityId;
class AssetLoadingThreadPool;
class Descriptor;
class Renderable;
struct AttachmentData;
struct FrameToken;
class Animation;
class OctreeData;
struct LightingPass;
class CmdBufferPool;
class AtmosphereComponent;
struct SSAOSettings;
struct BoneTransform;
class CmdBufferRegion;
struct BlurPass;
class StaticMesh;
class Device;
struct SubPass;
class SkinnedMesh;
class DescriptorSetBase;
class DedicatedDeviceMemoryAllocator;
class DescriptorSetData;
class SpirVData;
class OctreeSystem;
struct SkinnedVertex;
class SceneView;
class Frustum;
struct AssetId;
class SimpleMaterialData;
struct ToneMappingSettings;
struct KeepAlive;
class DescriptorSet;
class PointLightComponent;
class MaterialCompiler;
class EventHandler;
struct DeviceProperties;
struct SubPassData;
struct SceneRenderSubPass;
class AABB;
class SearchableFileSystemModel;
class AssetLoadingContext;
class ImageBase;
struct FrameGraphBufferId;
class Queue;
class FrameGraphFrameResources;
struct Bone;
class PushConstant;
class AccelerationStructure;
class RenderPass;
struct ShadowMapSettings;
struct SkinWeights;
class AssetStore;
class Skeleton;
class Camera;
struct ImageCopyInfo;
class Swapchain;
class LoadingJob;
struct DefaultRenderer;
struct SceneData;
class MeshData;
class Material;
class RayTracing;
struct GBufferPass;
class ShaderProgram;
class ImageData;
class SpotLightComponent;
struct ResourceUsageInfo;
struct FrameGraphMutableResourceId;
class ImageFormat;
class Octree;
class SQLiteAssetStore;
class DeviceMemoryHeap;
class FrameGraphRegion;
class AssetDependencies;
struct LoadableComponentRegister;
class FrameGraphResourceId;
class CmdBufferData;
class MaterialTemplateData;
class RenderPassRecorder;
class SkeletonInstance;
class AssetLoader;
class SQLiteFileSystemModel;
class FolderFileSystemModel;
class SubBufferBase;
struct InlineStorage;
struct FrameGraphMutableImageId;
struct Vertex;
class PhysicalDevice;
class DeviceResources;
class ComputeProgram;
struct ToneMappingPass;
class AnimationChannel;
class TransformableComponent;
class DeviceMemoryAllocator;
class TimeQuery;
class InlineDescriptor;
class AssetIdFactory;
class LoaderBase;
class FrameGraphDescriptorBinding;
class Sampler;
class Window;
struct BufferCreateInfo;
struct EmptyResource;
class FolderAssetStore;
class MaterialTemplate;
class ResourceFence;
struct EnclosingSphere;
struct LoadableComponentTypeInfo;
struct Monitor;
struct Attachment;
class Layout;
struct RendererSettings;
class LifetimeManager;
class Instance;
class DeviceMemory;
class OctreeNode;
class BufferBarrier;
class CmdBufferRecorder;
struct BlurSettings;
class GraphicPipeline;
class TransientBuffer;
struct AtmospherePass;
class FrameGraph;
class SpecializationData;
struct Contants;
}


namespace yave::detail {
struct VkStructInitializer;
class AssetPtrDataBase;
struct VkNull;
}


namespace yave::ecs {
class EntityPrefab;
class System;
struct IdReturnPolicy;
class Archetype;
struct IdComponentsReturnPolicy;
class EntityIdPool;
class EntityWorld;
class SparseIdSet;
struct ComponentsReturnPolicy;
class IdComponents;
class ComponentContainerBase;
class ComponentBoxBase;
class EntityId;
struct ComponentRuntimeInfo;
}


namespace yave::uniform {
struct Camera;
struct ShadowMapParams;
struct Surfel;
struct ToneMappingParams;
struct SH;
struct DirectionalLight;
struct SpotLight;
struct RayleighSky;
struct PointLight;
}


#endif // YAVE_UTILS_FORWARD_H