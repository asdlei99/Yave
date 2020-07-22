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
#ifndef YAVE_DEVICE_HANDLE_H
#define YAVE_DEVICE_HANDLE_H

#include "DeviceUtils.h"

namespace yave {

template<typename T>
class Handle {
	public:
		~Handle(); // LifetimeManager.h

		Handle() = default;

		Handle(const Handle&) = default;
		Handle& operator=(const Handle&) = default;

		Handle(Handle&& other) {
			swap(other);
		}

		Handle(T h) : _handle(std::move(h)) {
		}


		void swap(Handle& other) {
			std::swap(other._handle, _handle);
		}

		Handle& operator=(Handle&& other) {
			swap(other);
			return *this;
		}

		bool is_null() const {
			const T null = {};
			return _handle == null;
		}

		void clear() {
			_handle = {};
		}

		T& get() {
			return _handle;
		}

		const T& get() const {
			return _handle;
		}

		operator T&() {
			return _handle;
		}

		operator const T&() const {
			return _handle;
		}

		bool operator==(Handle h) const {
			return _handle == h._handle;
		}

		bool operator!=(Handle h) const {
			return !operator==(h);
		}

	private:
		T _handle = {};
};

}


#endif // YAVE_DEVICE_HANDLE_H
