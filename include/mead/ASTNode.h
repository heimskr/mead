#pragma once

#include "mead/Token.h"

#include <memory>
#include <vector>

namespace mead {
	enum class NodeType {
		Invalid,
		FunctionPrototype, FunctionDeclaration, FunctionDefinition, VariableDeclaration, VariableDefinition, Identifier, Type, Block,
		Const, Pointer, Reference,
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
			std::shared_ptr<ASTNode> add(Args &&...args) {
				auto new_node = std::make_shared<ASTNode>(std::forward<Args>(args)...);
				auto self = shared_from_this();
				new_node->reparent(self);
				return self;
			}

			template <typename... Args>
			static std::shared_ptr<ASTNode> make(Args &&...args) {
				return std::make_shared<ASTNode>(std::forward<Args>(args)...);
			}
	};

	using ASTNodePtr = std::shared_ptr<ASTNode>;
}
