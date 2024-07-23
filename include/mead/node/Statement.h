#pragma once

#include "mead/ASTNode.h"

#include <memory>

namespace mead {
	class BasicBlock;
	class Compiler;
	class Function;
	class Scope;

	class Statement: public ASTNode {
		protected:
			using ASTNode::ASTNode;

		public:
			virtual bool compile(Compiler &, Function &, Scope &, std::shared_ptr<BasicBlock>);
	};
}