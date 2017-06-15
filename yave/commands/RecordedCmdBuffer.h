/*******************************
Copyright (c) 2016-2017 Grégoire Angerand

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
#ifndef YAVE_COMMANDS_RECORDEDCMDBUFFER_H
#define YAVE_COMMANDS_RECORDEDCMDBUFFER_H

#include "CmdBuffer.h"

namespace yave {

struct AsyncSubmit {
	void operator()(const CmdBufferBase&) const {
	}
};

struct SyncSubmit {
	void operator()(const CmdBufferBase& b) const {
		b.wait();
	}
};

template<CmdBufferUsage Usage>
class RecordedCmdBuffer : public CmdBufferBase {

	CmdBufferBase&& end_recorder(CmdBufferRecorder<Usage>&& recorder) {
		if constexpr(Usage != CmdBufferUsage::Secondary) {
			recorder.end_renderpass();
		}
		recorder.vk_cmd_buffer().end();
		return std::move(recorder._cmd_buffer);
	}

	public:
		RecordedCmdBuffer() = default;

		RecordedCmdBuffer(CmdBufferRecorder<Usage>&& recorder) : RecordedCmdBuffer(end_recorder(std::move(recorder))) {
		}

		RecordedCmdBuffer(RecordedCmdBuffer&& other) : CmdBufferBase() {
			swap(other);
		}

		RecordedCmdBuffer& operator=(RecordedCmdBuffer&& other) {
			swap(other);
			return *this;
		}

		template<typename Policy>
		void submit(vk::Queue queue, const Policy& policy = Policy()) {
			CmdBufferBase::submit(queue);
			policy(*this);
		}

	private:
		friend class CmdBufferRecorder<Usage>;

		RecordedCmdBuffer(CmdBufferBase&& other) : CmdBufferBase(std::move(other)) {
		}
};

}

#endif // YAVE_COMMANDS_RECORDEDCMDBUFFER_H
