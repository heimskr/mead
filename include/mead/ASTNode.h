#pragma once

#include "mead/Token.h"

#include <map>
#include <memory>
#include <set>
#include <vector>

namespace mead {
	enum class NodeType {
		Invalid,
		FunctionPrototype, FunctionDeclaration, FunctionDefinition, VariableDeclaration, VariableDefinition, Identifier, Type, Block,
		Const, Pointer, Reference, Number, String,
		PrefixExpression, PostfixPrime, ConstructorExpression, UnaryExpression, CastExpression, SizeExpression, BinaryPrime,
		SingleNewExpression, ArrayNewExpression, EmptyPrime, EmptyStatement, ScopePrime, ArgumentsPrime, SubscriptPrime,
	};

	extern std::map<NodeType, const char *> nodeTypes;
	extern std::set<NodeType> primeTypes;

	class ASTNode: public std::enable_shared_from_this<ASTNode> {
		public:
			NodeType type{};
			Token token;
			std::weak_ptr<ASTNode> weakParent;
			std::vector<std::shared_ptr<ASTNode>> children;
			bool valid;

			ASTNode();
			ASTNode(NodeType type, Token token, std::weak_ptr<ASTNode> parent = {});

			std::shared_ptr<ASTNode> reparent(std::weak_ptr<ASTNode>);
			// void movePrimes();
			void debug(size_t padding = 0) const;

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

			const SourceLocation & location() const {
				return token.location;
			}
	};

	using ASTNodePtr = std::shared_ptr<ASTNode>;
}
