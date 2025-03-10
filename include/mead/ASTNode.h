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
		Const, Pointer, LReference, Number, String,
		PrefixIncrement, PrefixDecrement, PostfixIncrement, PostfixDecrement,
		ConstructorCall, FunctionCall, UnaryExpression, Cast, Sizeof, Binary, SingleNew, ArrayNew, Delete,
		EmptyPrime, EmptyStatement, Scope, Expressions, Subscript, AccessMember, Deref, GetAddress, UnaryPlus, UnaryMinus, LogicalNot, BitwiseNot,
		Assign, CompoundAssign, ConditionalExpression, Comma, ExpressionStatement, IfStatement, ReturnStatement,
	};

	extern std::map<NodeType, const char *> nodeTypes;

	class ASTNode: public std::enable_shared_from_this<ASTNode> {
		public:
			NodeType type{};
			Token token;
			std::weak_ptr<ASTNode> weakParent;
			std::vector<std::shared_ptr<ASTNode>> children;

			ASTNode();
			ASTNode(NodeType type, Token token, std::weak_ptr<ASTNode> parent = {});

			virtual ~ASTNode() = default;

			std::shared_ptr<ASTNode> reparent(std::weak_ptr<ASTNode>);
			void removeSelf();
			virtual std::ostream & debug(std::ostream & = std::cout, size_t padding = 0) const;
			std::string debugStr() const;

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

			inline size_t size() const {
				return children.size();
			}

			inline bool empty() const {
				return children.empty();
			}

			inline const auto & operator[](size_t index) const {
				return children[index];
			}

			inline const auto & at(size_t index) const {
				return children.at(index);
			}

			inline const auto & front() const {
				if (children.empty())
					throw std::out_of_range("ASTNode has no children");
				return children.front();
			}

			inline const auto & back() const {
				if (children.empty())
					throw std::out_of_range("ASTNode has no children");
				return children.back();
			}

			inline auto begin() {
				return children.begin();
			}

			inline auto end() {
				return children.end();
			}

			inline auto begin() const {
				return children.begin();
			}

			inline auto end() const {
				return children.end();
			}
	};

	using ASTNodePtr = std::shared_ptr<ASTNode>;
}

template <>
struct std::formatter<mead::NodeType> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &type, std::format_context &ctx) const {
		if (auto iter = mead::nodeTypes.find(type); iter != mead::nodeTypes.end())
			return std::format_to(ctx.out(), "{}", iter->second);

		return std::format_to(ctx.out(), "<NodeType:{}?>", static_cast<int>(type));
	}
};

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
