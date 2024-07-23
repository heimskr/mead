
#include "mead/IntermediateInstruction.h"

namespace mead {
	TwoWayJump::TwoWayJump(std::weak_ptr<BasicBlock> block, std::weak_ptr<BasicBlock> destination, ValuePtr condition):
		IntermediateInstruction(std::move(block)), destination(std::move(destination)), condition(std::move(condition)) {}
}
