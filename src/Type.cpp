#include "mead/Namespace.h"
#include "mead/Type.h"

#include <cassert>

namespace mead {
	Type::operator std::string() const {
		return std::format("{}", *this);
	}

	char IntType::getPrefix() const {
		return isSigned? 'i' : 'u';
	}

	IntType::IntType(int bit_width, bool is_signed):
		Type(getPrefix() + std::to_string(bit_width)), bitWidth(bit_width), isSigned(is_signed) {}

	std::string IntType::getName() const {
		return getPrefix() + std::to_string(bitWidth);
	}

	LLVMTypePtr IntType::toLLVM() const {
		return std::make_shared<LLVMIntType>(bitWidth);
	}

	std::format_context::iterator IntType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{}{}", getPrefix(), bitWidth);
	}

	VoidType::VoidType():
		Type("void") {}

	std::string VoidType::getName() const {
		return "void";
	}

	LLVMTypePtr VoidType::toLLVM() const {
		return std::make_shared<LLVMVoidType>();
	}

	std::format_context::iterator VoidType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "void");
	}

	PointerType::PointerType(TypePtr subtype):
		Type(subtype->getName() + '*'), subtype(std::move(subtype)) {}

	std::string PointerType::getName() const {
		return subtype->getName() + '*';
	}

	LLVMTypePtr PointerType::toLLVM() const {
		return std::make_shared<LLVMPointerType>(subtype->toLLVM());
	}

	std::format_context::iterator PointerType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{}*", subtype);
	}

	ClassType::ClassType(std::string name, std::weak_ptr<Namespace> owner):
		Type(std::move(name)), owner(std::move(owner)) {}

	Namespace & ClassType::getNamespace() const {
		auto locked = owner.lock();
		assert(locked);
		return *locked;
	}

	std::string ClassType::getName() const {
		return getNamespace().getFullName() + "::" + name;
	}

	LLVMTypePtr ClassType::toLLVM() const {
		assert(false);
		return {};
	}

	std::format_context::iterator ClassType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "class {}", getName());
	}
}
