#include "mead/node/VariableDefinition.h"
#include "mead/Scope.h"

#include <cassert>

namespace mead {
	VariableDefinition::VariableDefinition(Token token):
		Statement(NodeType::VariableDefinition, std::move(token)) {}

	const std::string & VariableDefinition::getVariableName() const {
		return at(0)->token.value;
	}

	ExpressionPtr VariableDefinition::getExpression() const {
		debug();
		assert(size() == 2);
		auto expression = std::dynamic_pointer_cast<Expression>(at(1));
		assert(expression);
		return expression;
	}

	bool VariableDefinition::compile(Compiler &compiler, Function &function, Scope &scope, std::shared_ptr<BasicBlock> block) {
		debug();

		ExpressionPtr expression = getExpression();

		const bool inserted = scope.insertVariable(getVariableName(), expression->getType(scope));
		if (!inserted) {
			return false;
		}

		return false;
	}
}
