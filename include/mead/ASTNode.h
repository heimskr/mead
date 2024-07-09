#pragma once

#include "mead/Token.h"

#include <format>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

namespace mead {
	enum class NodeType {
		Invalid,
		FunctionPrototype, FunctionDeclaration, FunctionDefinition, VariableDeclaration, VariableDefinition, Identifier, Type, Block,
		Const, Pointer, Reference, Number, String,
		PrefixExpression, Postfix, ConstructorExpression, UnaryExpression, CastExpression, SizeExpression, Binary, ClimbedBinary,
		SingleNewExpression, ArrayNewExpression, EmptyPrime, EmptyStatement, Scope, Arguments, Subscript,
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
			std::ostream & debug(std::ostream & = std::cout, size_t padding = 0) const;

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

template <>
struct std::formatter<mead::ASTNode> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &node, std::format_context &ctx) const {
		// Can't attach padding state to std::format_context. Here's a hack.
		std::stringstream ss;
		node.debug(ss);
		return std::format_to(ctx.out(), "{}", ss.str());
	}
};
