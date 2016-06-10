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
#ifndef N_SCRIPT_BYTECODEASSEMBLER_H
#define N_SCRIPT_BYTECODEASSEMBLER_H

#include "Bytecode.h"
#include <n/core/Array.h>

namespace n {
namespace script {

class BytecodeAssembler
{
	public:
		using RegisterType = uint;

		class Label
		{

			private:
				friend class BytecodeAssembler;

				Label(uint i) : index(i) {
				}

				uint index;
		};

		BytecodeAssembler();

		Label createLabel();



		BytecodeAssembler &addI(RegisterType to, RegisterType a, RegisterType b);
		BytecodeAssembler &mulI(RegisterType to, RegisterType a, RegisterType b);

		BytecodeAssembler &copy(RegisterType to, RegisterType from);
		BytecodeAssembler &set(RegisterType to, int64 value);


		BytecodeAssembler &jump(Label to);
		BytecodeAssembler &jumpNZ(RegisterType a, Label to);


		BytecodeAssembler &operator<<(BytecodeInstruction i);
		BytecodeAssembler &operator<<(const BytecodeAssembler &a);

		const core::Array<BytecodeInstruction> &getInstructions() const {
			return in;
		}

	private:
		BytecodeAssembler &ass(BytecodeInstruction i);

		core::Array<BytecodeInstruction> in;
};

}
}

#endif // N_SCRIPT_BYTECODEASSEMBLER_H
