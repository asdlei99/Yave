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

#include "DeviceMemory.h"
#include "DeviceMemoryHeapBase.h"

namespace yave {

DeviceMemory::DeviceMemory(DeviceMemoryHeapBase* heap, VkDeviceMemory memory, usize offset, usize size) :
		DeviceMemory(heap->device(), memory, offset, size) {
	_heap = heap;
}

DeviceMemory::DeviceMemory(DevicePtr dptr, VkDeviceMemory memory, usize offset, usize size) :
		DeviceLinked(dptr),
		_memory(memory),
		_offset(offset),
		_size(size) {
}

DeviceMemory::DeviceMemory(DeviceMemory&& other) {
	swap(other);
}

DeviceMemory& DeviceMemory::operator=(DeviceMemory&& other) {
	swap(other);
	return *this;
}

DeviceMemory::~DeviceMemory() {
	Y_TODO(right now we have to do device()->destroy to recycle memory properly, maybe we want to change that)
	y_always_assert(!device(), "DeviceMemory has not been freed");
}

void DeviceMemory::free() {
	y_profile();
	if(_memory && _heap) {
		_heap->free(*this);
	}
	// set device to nullptr
	struct Empty : DeviceLinked {} empty;
	DeviceLinked::swap(empty);

	y_debug_assert(is_null());
}

VkDeviceMemory DeviceMemory::vk_memory() const {
	return _memory;
}

usize DeviceMemory::vk_offset() const {
	return _offset;
}

usize DeviceMemory::vk_size() const {
	return _size;
}

DeviceMemoryHeapBase* DeviceMemory::heap() const {
	return _heap;
}

void DeviceMemory::swap(DeviceMemory& other) {
	DeviceLinked::swap(other);
	std::swap(_heap, other._heap);
	std::swap(_memory, other._memory);
	std::swap(_offset, other._offset);
	std::swap(_size, other._size);
}

bool DeviceMemory::operator==(const DeviceMemory& other) const {
	return
		device() == other.device() &&
		_memory == other._memory &&
		_offset == other._offset &&
		_size == other._size;
}

}
