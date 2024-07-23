#pragma once

#include "mead/node/Statement.h"

namespace mead {
	class Block: public Statement {
		public:
			Block(Token token);

			bool compile(Compiler &, Function &, Scope &, std::shared_ptr<BasicBlock>) final;
	};
}