#pragma once

#include "mead/node/Expression.h"

namespace mead {
	class FunctionCall: public Expression {
		public:
			FunctionCall(Token);

			ASTNodePtr getFunction() const;
			ASTNodePtr getArgs() const;

			std::shared_ptr<Type> getType(const Scope &) const override;
			bool isConstant(const Scope &) const override;
	};
}
