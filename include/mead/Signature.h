#pragma once

#include <memory>
#include <vector>

namespace mead {
	class Type;

	struct Signature {
		std::shared_ptr<Type> returnType;
		std::vector<std::shared_ptr<Type>> argumentTypes;
	};
}