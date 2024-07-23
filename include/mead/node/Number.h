#pragma once

#include "mead/node/Expression.h"

#include <concepts>
#include <string>


namespace mead {
	template <std::integral I>
#ifdef MEAD_PARSENUMBER_DECLARED
	I parseNumber(std::string_view view, int base);
#else
	I parseNumber(std::string_view view, int base = 10);
#endif
#define MEAD_PARSENUMBER_DECLARED

	template <std::floating_point F>
	F parseNumber(std::string_view view);


	class Number: public Expression {
		public:
			Number(Token);

			std::shared_ptr<Type> getType(const Scope &) const override;
			bool isConstant(const Scope &) const override;

			template <typename T>
			T getNumber() const {
				return parseNumber<T>(token.value);
			}
	};
}
