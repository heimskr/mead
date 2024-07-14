#include "mead/LLVMValue.h"

#include <format>

namespace mead {
	LLVMTypePtr LLVMIntValue::getType() const {
		return type;
	}

	LLVMIntValue::operator std::string() const {
		return std::format("{}", value);
	}
}
