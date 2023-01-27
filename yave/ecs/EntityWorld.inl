/*******************************
Copyright (c) 2016-2022 Grégoire Angerand

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
#ifndef YAVE_ECS_ENTITYWORLD_INL
#define YAVE_ECS_ENTITYWORLD_INL

#ifndef YAVE_ECS_ENTITYWORLD_H
#error this file should not be included directly

// Just to help the IDE
#include "EntityWorld.h"
#endif


namespace yave {
namespace ecs {

template<typename T>
std::unique_ptr<ComponentContainerBase> create_container() {
    return std::make_unique<ComponentContainer<traits::component_raw_type_t<T>>>();
}

template<typename T>
void create_component(EntityWorld& world, EntityId id) {
    world.add_component<T>(id);
}


template<typename... Args>
void RequiredComponents<Args...>::add_required_components(EntityWorld& world, EntityId id) {
    world.add_components<Args...>(id);
}


template<typename Component, typename SystemType, typename... Tail>
static inline void register_component_type_rec(System* system) {
    if(SystemType* s = dynamic_cast<SystemType*>(system)) {
        s->template register_component_type<Component>();
    }
    if constexpr(sizeof...(Tail)) {
        register_component_type_rec<Component, Tail...>(system);
    }
}

template<typename Component, typename... SystemTypes>
void SystemLinkedComponent<Component, SystemTypes...>::register_component_type(System* system) {
    register_component_type_rec<Component, SystemTypes...>(system);
}




template<typename T>
void ComponentContainerBase::add_required_components(EntityWorld& world, EntityId id) {
    unused(id);
    if constexpr(is_detected_v<detail::has_required_components_t, T>) {
        T::add_required_components(world, id);
    }
}




template<typename T>
ComponentBox<T>::ComponentBox(T t) : _component(std::move(t)) {
}

template<typename T>
ComponentRuntimeInfo ComponentBox<T>::runtime_info() const {
    return ComponentRuntimeInfo::create<T>();
}

template<typename T>
void ComponentBox<T>::add_to(EntityWorld& world, EntityId id) const {
    world.add_component<T>(id, _component);
}


template<typename F, typename FI>
void System::add_tick_function(F&& func, FI&& id_func) {
    _tick.emplace_back([=](EntityWorld& world) {
        if constexpr(!std::is_same_v<FI, nullptr_t>) {
            const auto ids = id_func();
            QueryBuilder builder(world, ids);
            func(builder);
        } else {
            QueryBuilder builder(world);
            func(builder);
        }

    });
}


}
}

#endif // YAVE_ECS_COMPONENTCONTAINER_H

