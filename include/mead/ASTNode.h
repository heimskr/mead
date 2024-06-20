#pragma once

#include "mead/Token.h"

#include <memory>
#include <vector>

namespace mead {
	enum class NodeType {
		Invalid,
		FunctionDeclaration, FunctionDefinition, VariableDefinition, Identifier,
	};

	class ASTNode: public std::enable_shared_from_this<ASTNode> {
		public:
			NodeType type{};
			Token token;
			std::weak_ptr<ASTNode> parent;
			std::vector<std::shared_ptr<ASTNode>> children;
			bool valid;

			ASTNode();
			ASTNode(NodeType type, Token token, std::weak_ptr<ASTNode> parent = {});

			std::shared_ptr<ASTNode> reparent(std::weak_ptr<ASTNode>);

			template <typename... Args>
			static std::shared_ptr<ASTNode> make(Args &&...args) {
				return std::make_shared<ASTNode>(std::forward<Args>(args)...);
			}
	};

	using ASTNodePtr = std::shared_ptr<ASTNode>;
}
