#include "mead/Type.h"

namespace mead {
	Type::operator std::string() const {
		return std::format("{}", *this);
	}

	char IntType::getPrefix() const {
		return isSigned? 'i' : 'u';
	}

	IntType::IntType(int bit_width, bool is_signed):
		bitWidth(bit_width), isSigned(is_signed) {}

	std::string IntType::getName() const {
		return getPrefix() + std::to_string(bitWidth);
	}

	LLVMTypePtr IntType::toLLVM() const {
		return std::make_shared<LLVMIntType>(bitWidth);
	}

	std::format_context::iterator IntType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{}{}", getPrefix(), bitWidth);
	}

	VoidType::VoidType() = default;

	std::string VoidType::getName() const {
		return "void";
	}

	LLVMTypePtr VoidType::toLLVM() const {
		return std::make_shared<LLVMVoidType>();
	}

	std::format_context::iterator VoidType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "void");
	}
}
