#pragma once

#include "mead/Symbol.h"

#include <memory>
#include <string>
#include <vector>

namespace mead {
	class Type;

	class Function: public Symbol {
		private:
			std::shared_ptr<Type> returnType;
			std::vector<std::shared_ptr<Type>> argumentTypes;

		public:
			Function(std::string name, std::shared_ptr<Type> return_type, std::vector<std::shared_ptr<Type>> argument_types);

			inline const auto & getReturnType() const { return returnType; }
			inline const auto & getArgumentTypes() const { return argumentTypes; }
	};
}
