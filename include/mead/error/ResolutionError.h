#pragma once

#include <format>
#include <stdexcept>

namespace mead {
	struct ResolutionError: std::runtime_error {
		std::string symbol;

		ResolutionError(std::string symbol):
			std::runtime_error(std::format("Failed to resolve \"{}\"", symbol)), symbol(std::move(symbol)) {}
	};
}
