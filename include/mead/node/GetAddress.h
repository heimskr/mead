#pragma once

#include "mead/node/Expression.h"

namespace mead {
	class GetAddress: public Expression {
		public:
			GetAddress(Token);

			std::shared_ptr<Type> getType(const Scope &) const override;
			bool isConstant(const Scope &) const;
	};
}
