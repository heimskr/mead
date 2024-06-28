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
		{NodeType::PrefixExpression, "PrefixExpression"},
		{NodeType::PostfixPrime, "PostfixPrime"},
		{NodeType::ConstructorExpression, "ConstructorExpression"},
		{NodeType::UnaryExpression, "UnaryExpression"},
		{NodeType::CastExpression, "CastExpression"},
		{NodeType::SizeExpression, "SizeExpression"},
		{NodeType::SingleNewExpression, "SingleNewExpression"},
		{NodeType::ArrayNewExpression, "ArrayNewExpression"},
		{NodeType::EmptyPrime, "EmptyPrime"},
		{NodeType::EmptyStatement, "EmptyStatement"},
		{NodeType::ScopePrime, "ScopePrime"},
		{NodeType::ArgumentsPrime, "ArgumentsPrime"},
		{NodeType::SubscriptPrime, "SubscriptPrime"},
		{NodeType::Number, "Number"},
		{NodeType::String, "String"},
	};

	ASTNode::ASTNode():
		valid(false) {}

	ASTNode::ASTNode(NodeType type, Token token, std::weak_ptr<ASTNode> parent):
		type(type), token(std::move(token)), parent(std::move(parent)), valid(true) {}

	std::shared_ptr<ASTNode> ASTNode::reparent(std::weak_ptr<ASTNode> new_parent) {
		assert(parent.expired());
		parent = std::move(new_parent);
		auto self = shared_from_this();

		if (auto locked = parent.lock()) {
			locked->children.push_back(self);
		}

		return self;
	}

	void ASTNode::debug(size_t padding) const {
		std::println("{}{}: {}", std::string(padding, ' '), nodeTypes.at(type), token);
		for (const auto &child: children) {
			child->debug(padding + 2);
		}
	}
}
