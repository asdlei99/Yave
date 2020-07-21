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

#include "DeviceUtils.h"

#include "Device.h"

namespace yave {

VkDevice vk_main_device() {
	return main_device()->vk_device();
}

CmdBuffer<CmdBufferUsage::Disposable> create_disposable_cmd_buffer() {
	return main_device()->create_disposable_cmd_buffer();
}

DeviceMemoryAllocator& device_allocator() {
	return main_device()->allocator();
}

DescriptorSetAllocator& descriptor_set_allocator() {
	return main_device()->descriptor_set_allocator();
}

const Queue& graphic_queue() {
	return main_device()->graphic_queue();
}

const DeviceResources& device_resources() {
	return main_device()->device_resources();
}

const DeviceProperties& device_properties() {
	return main_device()->device_properties();
}

LifetimeManager& lifetime_manager() {
	return main_device()->lifetime_manager();
}

const VkAllocationCallbacks* vk_allocation_callbacks() {
	return main_device()->vk_allocation_callbacks();
}

}
