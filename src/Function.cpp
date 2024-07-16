#include "mead/Function.h"
#include "mead/Type.h"
#include "mead/Util.h"

namespace mead {
	Function::Function(std::string name, std::shared_ptr<Type> return_type, std::vector<std::shared_ptr<Type>> argument_types):
		Symbol(std::move(name)), returnType(std::move(return_type)), argumentTypes(std::move(argument_types)) {}

	std::format_context::iterator Function::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "fn {}({}) -> {}", name, join(argumentTypes), returnType);
	}
}
