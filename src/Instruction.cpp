#include "mead/util/Demangle.h"
#include "mead/Instruction.h"

namespace mead {
	Instruction::Instruction(std::weak_ptr<BasicBlock> block):
		block(std::move(block)) {}

	std::string Instruction::toString() const {
		return '[' + DEMANGLE(*this) + ']';
	}
}
