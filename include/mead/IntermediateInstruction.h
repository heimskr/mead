#pragma once

#include "mead/Instruction.h"
#include "mead/Value.h"

namespace mead {
	/** Base class for special instructions that don't directly correspond to an LLVM instruction.
	 *  Typically, they require special logic (with potential side effects) to replace them with LLVM instructions. */
	class IntermediateInstruction: public Instruction {
		protected:
			using Instruction::Instruction;
	};

	/** Represents an instruction that jumps to another basic block that eventually returns control to the next instruction.
	 *  This will eventually cause its parent basic block to be split. */
	class TwoWayJump: public IntermediateInstruction {
		private:
			std::weak_ptr<BasicBlock> destination;
			/** Can be null. */
			ValuePtr condition;

		public:
			TwoWayJump(std::weak_ptr<BasicBlock> block, std::weak_ptr<BasicBlock> destination, ValuePtr condition);
	};
}
