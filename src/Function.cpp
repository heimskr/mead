#include "mead/Function.h"

namespace mead {
	Function::Function(std::string name, std::shared_ptr<Type> return_type, std::vector<std::shared_ptr<Type>> argument_types):
		Symbol(std::move(name)), returnType(std::move(return_type)), argumentTypes(std::move(argument_types)) {}
}
