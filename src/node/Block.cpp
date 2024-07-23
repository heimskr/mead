#include "mead/node/Block.h"
#include "mead/Logging.h"
#include "mead/Scope.h"

#include <cassert>

namespace mead {
	Block::Block(Token token):
		Statement(NodeType::Block, std::move(token)) {}

	bool Block::compile(Compiler &compiler, Function &function, Scope &scope, std::shared_ptr<BasicBlock> basic_block) {
		if (empty()) {
			return true;
		}

		for (const ASTNodePtr &node : *this) {
			auto statement = std::dynamic_pointer_cast<Statement>(node);
			if (!statement) {
				ERROR("Bad statement:\n");
				node->debug(std::cerr, 4) << "\n";
			}
			assert(statement);
			if (!statement->compile(compiler, function, scope, basic_block)) {
				return false;
			}
		}

		return true;
	}
}
