#pragma once

#include "mead/node/Expression.h"
#include "mead/node/Statement.h"

#include <memory>

namespace mead {
	class Expression;

	class VariableDefinition: public Statement {
		public:
			VariableDefinition(Token token);

			const std::string & getVariableName() const;
			std::shared_ptr<Expression> getExpression() const;
			bool compile(Compiler &, Function &, Scope &, std::shared_ptr<BasicBlock>) final;
	};
}
