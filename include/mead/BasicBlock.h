#pragma once

#include <list>
#include <memory>
#include <set>

#include "mead/util/WeakSet.h"

namespace mead {
	class Function;
	class LLVMInstruction;

	class BasicBlock: public std::enable_shared_from_this<BasicBlock> {
		private:
			Function *parent = nullptr;
			WeakSet<BasicBlock> in;
			WeakSet<BasicBlock> out;
			std::list<std::shared_ptr<LLVMInstruction>> instructions;

		public:
			BasicBlock(Function &parent);

			void connectTo(BasicBlock &);

			/** Removes this block from the in/out sets of the other and vice versa. */
			void disconnect(BasicBlock &);

			template <typename T, typename... Args>
			requires std::derived_from<T, LLVMInstruction>
			std::shared_ptr<T> add(Args &&...args) {
				auto out = std::make_shared<T>(std::forward<Args>(args)...);
				instructions.emplace_back(out);
				return out;
			}
	};
}
