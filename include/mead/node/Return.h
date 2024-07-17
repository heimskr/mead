#pragma once

#include "mead/node/Expression.h"
#include "mead/node/Statement.h"

#include <memory>

namespace mead {
	class Expression;

	class Return: public Statement {
		public:
			Return(Token token);

			std::shared_ptr<Expression> getExpression() const;
	};
}
