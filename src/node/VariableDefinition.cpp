#include "mead/node/VariableDefinition.h"

#include <cassert>

namespace mead {
	VariableDefinition::VariableDefinition(Token token):
		Statement(NodeType::VariableDefinition, std::move(token)) {}

	std::shared_ptr<Expression> VariableDefinition::getExpression() const {
		debug();
		assert(size() == 2);
		auto expression = std::dynamic_pointer_cast<Expression>(at(1));
		assert(expression);
		return expression;
	}
}
