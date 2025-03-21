#include "mead/ASTNode.h"

#include <cassert>
#include <print>

namespace mead {
	std::map<NodeType, const char *> nodeTypes{
		{NodeType::Invalid, "Invalid"},
		{NodeType::FunctionPrototype, "FunctionPrototype"},
		{NodeType::FunctionDeclaration, "FunctionDeclaration"},
		{NodeType::FunctionDefinition, "FunctionDefinition"},
		{NodeType::FunctionCall, "FunctionCall"},
		{NodeType::VariableDeclaration, "VariableDeclaration"},
		{NodeType::VariableDefinition, "VariableDefinition"},
		{NodeType::Identifier, "Identifier"},
		{NodeType::Type, "Type"},
		{NodeType::Block, "Block"},
		{NodeType::Const, "Const"},
		{NodeType::Pointer, "Pointer"},
		{NodeType::LReference, "LReference"},
		{NodeType::PrefixIncrement, "PrefixIncrement"},
		{NodeType::PrefixDecrement, "PrefixDecrement"},
		{NodeType::PostfixIncrement, "PostfixIncrement"},
		{NodeType::PostfixDecrement, "PostfixDecrement"},
		{NodeType::ConstructorCall, "Constructor"},
		{NodeType::UnaryExpression, "Unary"},
		{NodeType::Cast, "Cast"},
		{NodeType::Sizeof, "Sizeof"},
		{NodeType::SingleNew, "SingleNew"},
		{NodeType::ArrayNew, "ArrayNew"},
		{NodeType::EmptyPrime, "EmptyPrime"},
		{NodeType::EmptyStatement, "EmptyStatement"},
		{NodeType::Scope, "Scope"},
		{NodeType::Expressions, "Expressions"},
		{NodeType::Subscript, "Subscript"},
		{NodeType::Number, "Number"},
		{NodeType::String, "String"},
		{NodeType::Binary, "Binary"},
		{NodeType::AccessMember, "AccessMember"},
		{NodeType::Deref, "Deref"},
		{NodeType::GetAddress, "GetAddress"},
		{NodeType::UnaryPlus, "UnaryPlus"},
		{NodeType::UnaryMinus, "UnaryMinus"},
		{NodeType::LogicalNot, "LogicalNot"},
		{NodeType::BitwiseNot, "BitwiseNot"},
		{NodeType::Delete, "Delete"},
		{NodeType::Assign, "Assign"},
		{NodeType::CompoundAssign, "CompoundAssign"},
		{NodeType::ConditionalExpression, "ConditionalExpression"},
		{NodeType::Comma, "Comma"},
		{NodeType::ExpressionStatement, "ExpressionStatement"},
		{NodeType::IfStatement, "IfStatement"},
		{NodeType::ReturnStatement, "ReturnStatement"},
	};

	ASTNode::ASTNode() = default;

	ASTNode::ASTNode(NodeType type, Token token, std::weak_ptr<ASTNode> parent):
		type(type), token(std::move(token)), weakParent(std::move(parent)) {}

	std::shared_ptr<ASTNode> ASTNode::reparent(std::weak_ptr<ASTNode> new_parent) {
		auto self = shared_from_this();

		if (new_parent.lock() == weakParent.lock()) {
			return self;
		}

		// Remove self from parent's children if necessary
		removeSelf();

		weakParent = std::move(new_parent);

		if (ASTNodePtr parent = weakParent.lock()) {
			parent->children.push_back(self);
		}

		return self;
	}

	void ASTNode::removeSelf() {
		if (ASTNodePtr parent = weakParent.lock()) {
			auto self = shared_from_this();
			for (auto iter = parent->children.begin(); iter != parent->children.end(); ++iter) {
				if (*iter == self) {
					parent->children.erase(iter);
					break;
				}
			}
		}

		weakParent.reset();
	}

	std::ostream & ASTNode::debug(std::ostream &stream, size_t padding) const {
		std::string node_name;
		if (auto iter = nodeTypes.find(type); iter != nodeTypes.end())
			node_name = iter->second;
		else
			node_name = std::format("\x1b[31m[NodeType={}?]\x1b[39m", static_cast<int>(type));
		std::println(stream, "{}{}: {}", std::string(padding, ' '), node_name, token);
		for (const auto &child: children) {
			child->debug(stream, padding + 2);
		}
		return stream;
	}

	std::string ASTNode::debugStr() const {
		std::stringstream ss;
		debug(ss);
		return ss.str();
	}
}
