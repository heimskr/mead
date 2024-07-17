#include "mead/node/Return.h"

#include <cassert>

namespace mead {
	Return::Return(Token token):
		Statement(NodeType::ReturnStatement, std::move(token)) {}

	std::shared_ptr<Expression> Return::getExpression() const {
		assert(size() == 1);
		auto expression = std::dynamic_pointer_cast<Expression>(front());
		assert(expression);
		return expression;
	}
}
