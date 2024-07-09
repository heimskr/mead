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
		{NodeType::PostfixPrime, "Postfix"},
		{NodeType::ConstructorExpression, "Constructor"},
		{NodeType::UnaryExpression, "Unary"},
		{NodeType::CastExpression, "Cast"},
		{NodeType::SizeExpression, "Size"},
		{NodeType::SingleNewExpression, "SingleNew"},
		{NodeType::ArrayNewExpression, "ArrayNew"},
		{NodeType::EmptyPrime, "Empty"},
		{NodeType::EmptyStatement, "EmptyStatement"},
		{NodeType::ScopePrime, "Scope"},
		{NodeType::ArgumentsPrime, "Arguments"},
		{NodeType::SubscriptPrime, "Subscript"},
		{NodeType::Number, "Number"},
		{NodeType::String, "String"},
		{NodeType::BinaryPrime, "Binary"},
	};

	std::set<NodeType> primeTypes{
		NodeType::PostfixPrime,
		NodeType::EmptyPrime,
		NodeType::ScopePrime,
		NodeType::ArgumentsPrime,
		NodeType::SubscriptPrime,
		NodeType::BinaryPrime,
	};

	ASTNode::ASTNode():
		valid(false) {}

	ASTNode::ASTNode(NodeType type, Token token, std::weak_ptr<ASTNode> parent):
		type(type), token(std::move(token)), weakParent(std::move(parent)), valid(true) {}

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

	// void ASTNode::movePrimes() {
	// 	auto parent = weakParent.lock();

	// 	if (!children.empty() && children.back()->type == NodeType::EmptyPrime) {
	// 		children.pop_back();
	// 	}

	// 	const auto old_children = children;

	// 	if (parent) {
	// 		std::vector<ASTNodePtr> to_reparent;

	// 		for (const ASTNodePtr &child : children) {
	// 			if (primeTypes.contains(child->type)) {
	// 				to_reparent.push_back(child);
	// 			}
	// 		}

	// 		for (const ASTNodePtr &child : to_reparent) {
	// 			child->reparent(parent);
	// 		}
	// 	}

	// 	for (const ASTNodePtr &child : old_children) {
	// 		child->movePrimes();
	// 	}
	// }

	void ASTNode::debug(size_t padding) const {
		std::println("{}{}: {}", std::string(padding, ' '), nodeTypes.at(type), token);
		for (const auto &child: children) {
			child->debug(padding + 2);
		}
	}
}
