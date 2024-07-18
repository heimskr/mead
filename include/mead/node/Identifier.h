#pragma once

#include "mead/node/Expression.h"

namespace mead {
	class Identifier: public Expression {
		public:
			Identifier(Token);

			std::shared_ptr<Type> getType(const Scope &) const override;
			bool isConstant(const Scope &) const override;

			inline const std::string & getIdentifier() const { return token.value; }
	};
}
