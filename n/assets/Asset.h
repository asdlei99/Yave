/*******************************
Copyright (C) 2013-2015 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/

#ifndef N_ASSETS_ASSET_H
#define N_ASSETS_ASSET_H

#include <n/concurent/MultiThreadPtr.h>

namespace n {
namespace assets {

template<typename T>
using AssetPtr = concurent::MultiThreadPtr<T *>;

template<typename T, typename LoadPolicy>
class AssetBuffer;

template<typename T>
class Asset
{
	public:
		Asset() : ptr(0) {
		}

		T &operator->() {
			return **ptr;
		}

		const T &operator->() const {
			return **ptr;
		}

		T &operator*() {
			return **ptr;
		}

		const T &operator*() const {
			return **ptr;
		}

		bool isValid() const {
			return *ptr;
		}

	private:
		template<typename U, typename P>
		friend class AssetBuffer;

		Asset(AssetPtr<T> p) : ptr(p) {
		}

		AssetPtr<T> ptr;
};

}
}

#endif // N_ASSETS_ASSET_H
