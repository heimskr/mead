#include "mead/node/FunctionCall.h"

#include <cassert>

namespace mead {
	FunctionCall::FunctionCall(Token token):
		Expression(NodeType::FunctionCall, std::move(token)) {}

	ASTNodePtr FunctionCall::getFunction() const {
		return children.at(0);
	}

	ASTNodePtr FunctionCall::getArgs() const {
		return children.at(1);
	}

	std::shared_ptr<Type> FunctionCall::getType(const Scope &) const {
		ASTNodePtr function = getFunction();

		if (function->type == NodeType::Identifier) {

		}

		return {};
	}
}
