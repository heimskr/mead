#include "mead/LLVMInstruction.h"

namespace mead {
	LLVMRet::LLVMRet(std::weak_ptr<BasicBlock> block, LLVMValuePtr value):
		LLVMInstruction(std::move(block)), value(std::move(value)) {}

	LLVMThreeReg::LLVMThreeReg(std::weak_ptr<BasicBlock> block, LLVMValuePtr left, LLVMValuePtr right, LLVMValuePtr result):
		LLVMInstruction(std::move(block)), left(std::move(left)), right(std::move(right)), result(std::move(result)) {}

	void LLVMThreeReg::assertValid() const {
		assert(left);
		assert(right);
		assert(result);
		auto left_type = left->getType();
		auto right_type = right->getType();
		assert(left_type);
		assert(right_type);
		assert(*left_type == *right_type);
	}

	std::string LLVMThreeReg::toString() const {
		assertValid();
		return std::format("{} = {} {} {}, {}", result->toString(), getKeyword(), left->getType(), left->toString(), right->toString());
	}
}
