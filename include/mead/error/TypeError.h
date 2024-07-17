#pragma once

#include <format>
#include <memory>
#include <stdexcept>

namespace mead {
	class Type;

	struct TypeError: std::runtime_error {
		std::shared_ptr<Type> from;
		std::shared_ptr<Type> to;

		TypeError(std::shared_ptr<Type> from, std::shared_ptr<Type> to);
	};
}
