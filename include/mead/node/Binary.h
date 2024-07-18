#pragma once

#include "mead/node/Expression.h"

namespace mead {
	class Binary: public Expression {
		public:
			Binary(Token);

			ExpressionPtr getLHS() const;
			ExpressionPtr getRHS() const;

			std::shared_ptr<Type> getType(const Scope &) const override;
			bool isConstant(const Scope &) const override;
	};
}
