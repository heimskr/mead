#include "mead/Type.h"
#include "mead/Variable.h"

namespace mead {
	Variable::Variable(std::string name, std::shared_ptr<Type> type):
		Symbol(std::move(name)), type(std::move(type)) {}

	std::format_context::iterator Variable::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{}: {}", name, type);
	}
}
