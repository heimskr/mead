#pragma once

#include "mead/Formattable.h"
#include "mead/Symbol.h"

#include <memory>
#include <string>

namespace mead {
	class Type;

	class Variable: public Symbol, public Formattable {
		private:
			std::shared_ptr<Type> type;

		public:
			Variable(std::string name, std::shared_ptr<Type> type);

			std::format_context::iterator formatTo(std::format_context &) const override;

			inline const auto & getName() const { return name; }
			inline const auto & getType() const { return type; }
	};

	using VariablePtr = std::shared_ptr<Variable>;
}
