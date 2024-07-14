#include "mead/LLVMType.h"
#include "mead/Util.h"

#include <format>

namespace mead {
	LLVMType::operator std::string() const {
		return std::format("{}", *this);
	}

	LLVMIntType::LLVMIntType(int bit_width):
		bitWidth(bit_width) {}

	bool LLVMIntType::operator==(const LLVMType &other) const {
		if (this == &other)
			return true;

		if (auto *cast = dynamic_cast<const LLVMIntType *>(&other))
			return cast->bitWidth == bitWidth;

		return false;
	}

	std::format_context::iterator LLVMIntType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "i{}", bitWidth);
	}

	LLVMArrayType::LLVMArrayType(int count, LLVMTypePtr subtype):
		count(count), subtype(std::move(subtype)) {}

	bool LLVMArrayType::operator==(const LLVMType &other) const {
		if (this == &other)
			return true;

		if (auto *cast = dynamic_cast<const LLVMArrayType *>(&other))
			return cast->count == count && *cast->subtype == *subtype;

		return false;
	}

	std::format_context::iterator LLVMArrayType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "[{} x {}]", count, subtype);
	}

	LLVMStructType::LLVMStructType(std::vector<LLVMTypeWeakPtr> subtypes):
		subtypes(std::move(subtypes)) {}

	bool LLVMStructType::operator==(const LLVMType &other) const {
		if (this == &other)
			return true;

		if (auto *cast = dynamic_cast<const LLVMStructType *>(&other)) {
			if (cast->subtypes.size() != subtypes.size())
				return false;

			for (size_t i = 0; i < subtypes.size(); ++i) {
				auto subtype = subtypes[i].lock();
				auto other_subtype = cast->subtypes[i].lock();
				assert(subtype);
				assert(other_subtype);
				if (*subtype != *other_subtype)
					return false;
			}

			return true;
		}

		return false;
	}

	std::format_context::iterator LLVMStructType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{{{}}}", join(subtypes));
	}

	LLVMPointerType::LLVMPointerType(LLVMTypePtr subtype):
		subtype(std::move(subtype)) {}

	bool LLVMPointerType::operator==(const LLVMType &other) const {
		if (this == &other)
			return true;

		if (auto *cast = dynamic_cast<const LLVMPointerType *>(&other)) {
			if (cast->subtype && subtype)
				return *cast->subtype == *subtype;
			return !cast->subtype == !subtype;
		}

		return false;
	}

	std::format_context::iterator LLVMPointerType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "ptr");
	}
}
