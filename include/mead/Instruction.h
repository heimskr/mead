#pragma once

#include <memory>
#include <string>

namespace mead {
	class BasicBlock;

	class Instruction {
		protected:
			std::weak_ptr<BasicBlock> block;

			Instruction(std::weak_ptr<BasicBlock> block);

		public:
			virtual ~Instruction() = default;

			virtual std::string toString() const;
	};
}
