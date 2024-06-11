#pragma once

#include "mead/Token.h"

#include <memory>
#include <vector>

namespace mead {
	enum class NodeType {
		Invalid,
		FunctionDeclaration, FunctionDefinition, VariableDefinition,
	};

	class ASTNode {
		public:
			NodeType type{};
			Token token;
			std::weak_ptr<ASTNode> parent;
			std::vector<std::shared_ptr<ASTNode>> children;

			ASTNode(NodeType type, Token token, std::weak_ptr<ASTNode> parent);
	};
}
