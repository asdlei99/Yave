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
#ifndef YAVE_GRAPHICS_IMAGES_IMAGEBASE_H
#define YAVE_GRAPHICS_IMAGES_IMAGEBASE_H

#include <yave/device/DeviceLinked.h>
#include <yave/device/Handle.h>
#include <yave/graphics/memory/DeviceMemory.h>

#include "ImageUsage.h"
#include "ImageData.h"

#include <yave/utils/traits.h>

namespace yave {

class ImageBase : NonCopyable {
	public:
		DevicePtr device() const;
		bool is_null() const;

		VkImage vk_image() const;
		VkImageView vk_view() const;

		const DeviceMemory& device_memory() const;

		const math::Vec3ui& image_size() const;
		usize mipmaps() const;

		usize layers() const;

		ImageFormat format() const;
		ImageUsage usage() const;

	protected:
		ImageBase() = default;
		ImageBase(ImageBase&&) = default;
		ImageBase& operator=(ImageBase&&) = default;

		ImageBase(DevicePtr dptr, ImageFormat format, ImageUsage usage, const math::Vec3ui& size, ImageType type = ImageType::TwoD, usize layers = 1, usize mips = 1);
		ImageBase(DevicePtr dptr, ImageUsage usage, ImageType type, const ImageData& data);


		math::Vec3ui _size;
		u32 _layers = 1;
		u32 _mips = 1;

		ImageFormat _format;
		ImageUsage _usage = ImageUsage::None;

		Handle<DeviceMemory> _memory;

		Handle<VkImage> _image;
		Handle<VkImageView> _view;
};

static_assert(is_safe_base<ImageBase>::value);

}

#endif // YAVE_GRAPHICS_IMAGES_IMAGEBASE_H
