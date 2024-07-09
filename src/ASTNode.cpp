#include "mead/ASTNode.h"

#include <cassert>
#include <print>

namespace mead {
	std::map<NodeType, const char *> nodeTypes{
		{NodeType::Invalid, "Invalid"},
		{NodeType::FunctionPrototype, "FunctionPrototype"},
		{NodeType::FunctionDeclaration, "FunctionDeclaration"},
		{NodeType::FunctionDefinition, "FunctionDefinition"},
		{NodeType::VariableDeclaration, "VariableDeclaration"},
		{NodeType::VariableDefinition, "VariableDefinition"},
		{NodeType::Identifier, "Identifier"},
		{NodeType::Type, "Type"},
		{NodeType::Block, "Block"},
		{NodeType::Const, "Const"},
		{NodeType::Pointer, "Pointer"},
		{NodeType::Reference, "Reference"},
		{NodeType::PrefixExpression, "Prefix"},
		{NodeType::Postfix, "Postfix"},
		{NodeType::ConstructorExpression, "Constructor"},
		{NodeType::UnaryExpression, "Unary"},
		{NodeType::CastExpression, "Cast"},
		{NodeType::SizeExpression, "Size"},
		{NodeType::SingleNewExpression, "SingleNew"},
		{NodeType::ArrayNewExpression, "ArrayNew"},
		{NodeType::EmptyPrime, "EmptyPrime"},
		{NodeType::EmptyStatement, "EmptyStatement"},
		{NodeType::Scope, "Scope"},
		{NodeType::Arguments, "Arguments"},
		{NodeType::Subscript, "Subscript"},
		{NodeType::Number, "Number"},
		{NodeType::String, "String"},
		{NodeType::Binary, "Binary"},
		{NodeType::ClimbedBinary, "ClimbedBinary"},
	};

	std::set<NodeType> primeTypes{
		NodeType::Postfix,
		NodeType::EmptyPrime,
		NodeType::Scope,
		NodeType::Arguments,
		NodeType::Subscript,
		NodeType::Binary,
	};

	ASTNode::ASTNode():
		valid(false) {}

	ASTNode::ASTNode(NodeType type, Token token, int precedence, std::weak_ptr<ASTNode> parent):
		type(type), token(std::move(token)), weakParent(std::move(parent)), precedence(precedence), valid(true) {}

	std::shared_ptr<ASTNode> ASTNode::reparent(std::weak_ptr<ASTNode> new_parent) {
		auto self = shared_from_this();

		if (new_parent.lock() == weakParent.lock()) {
			return self;
		}

		// Remove self from parent's children if necessary
		if (auto parent = weakParent.lock()) {
			for (auto iter = parent->children.begin(); iter != parent->children.end(); ++iter) {
				if (*iter == self) {
					parent->children.erase(iter);
					break;
				}
			}
		}

		weakParent = std::move(new_parent);

		if (auto parent = weakParent.lock()) {
			parent->children.push_back(self);
		}

		return self;
	}

	std::ostream & ASTNode::debug(std::ostream &stream, size_t padding) const {
		std::println(stream, "{}{}: {}", std::string(padding, ' '), nodeTypes.at(type), token);
		for (const auto &child: children) {
			child->debug(stream, padding + 2);
		}
		return stream;
	}
}
