#pragma once

#include "mead/node/Expression.h"

namespace mead {
	class Dereference: public Expression {
		public:
			Dereference(Token);

			std::shared_ptr<Type> getType(const Scope &) const override;
	};
}
