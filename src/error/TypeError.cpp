#include "mead/error/TypeError.h"
#include "mead/Type.h"

namespace mead {
	TypeError::TypeError(std::shared_ptr<Type> from, std::shared_ptr<Type> to):
		std::runtime_error(std::format("Failed to convert type {} to {}", from, to)), from(std::move(from)), to(std::move(to)) {}
}
