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
#ifndef EDITOR_WIDGETS_ENTITYVIEW_H
#define EDITOR_WIDGETS_ENTITYVIEW_H

#include <editor/ui/Widget.h>

#include <yave/ecs/ecs.h>
#include <yave/ecs/ComponentRuntimeInfo.h>

namespace editor {

class EntityView final : public Widget, public ContextLinked {

	public:
		EntityView(ContextPtr cptr);

	private:
		void paint_ui(CmdBufferRecorder&, const FrameToken&) override;

		void paint_view();

		ecs::EntityId _hovered;

		core::Vector<std::pair<core::String, ecs::ComponentRuntimeInfo>> _all_components;
};

}

#endif // EDITOR_WIDGETS_ENTITYVIEW_H
