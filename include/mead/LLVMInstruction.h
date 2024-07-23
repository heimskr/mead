#pragma once

#include "mead/Instruction.h"
#include "mead/LLVMValue.h"

namespace mead {
	class LLVMInstruction: public Instruction {
		protected:
			using Instruction::Instruction;
	};

	class LLVMRet: public LLVMInstruction {
		private:
			ValuePtr value;

		public:
			LLVMRet(std::weak_ptr<BasicBlock> block, LLVMValuePtr value);
	};

	class LLVMThreeReg: public LLVMInstruction {
		protected:
			LLVMValuePtr left;
			LLVMValuePtr right;
			LLVMValuePtr result;

			virtual std::string getKeyword() const = 0;
			virtual void assertValid() const;

		public:
			LLVMThreeReg(std::weak_ptr<BasicBlock> block, LLVMValuePtr left, LLVMValuePtr right, LLVMValuePtr result);

			std::string toString() const override;
	};

	class LLVMAdd: public LLVMThreeReg {
		protected:
			std::string getKeyword() const final { return "add"; }

		public:
			using LLVMThreeReg::LLVMThreeReg;
	};
}
