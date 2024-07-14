#include "mead/LLVMType.h"
#include "mead/Util.h"

#include <format>

namespace mead {
	LLVMIntType::LLVMIntType(int bit_width):
		bitWidth(bit_width) {}

	LLVMIntType::operator std::string() const {
		return std::format("i{}", bitWidth);
	}

	bool LLVMIntType::operator==(const LLVMType &other) const {
		if (this == &other)
			return true;

		if (auto *cast = dynamic_cast<const LLVMIntType *>(&other))
			return cast->bitWidth == bitWidth;

		return false;
	}

	LLVMArrayType::LLVMArrayType(int count, LLVMTypePtr subtype):
		count(count), subtype(std::move(subtype)) {}

	LLVMArrayType::operator std::string() const {
		return std::format("{} x {}", count, subtype);
	}

	bool LLVMArrayType::operator==(const LLVMType &other) const {
		if (this == &other)
			return true;

		if (auto *cast = dynamic_cast<const LLVMArrayType *>(&other))
			return cast->count == count && *cast->subtype == *subtype;

		return false;
	}

	LLVMStructType::LLVMStructType(std::vector<LLVMTypeWeakPtr> subtypes):
		subtypes(std::move(subtypes)) {}

	LLVMStructType::operator std::string() const {
		return std::format("{{{}}}", join(subtypes));
	}

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

	LLVMPointerType::LLVMPointerType(LLVMTypePtr subtype):
		subtype(std::move(subtype)) {}

	LLVMPointerType::operator std::string() const {
		return "ptr";
	}

	bool LLVMPointerType::operator==(const LLVMType &other) const {
		if (this == &other)
			return true;

		if (auto *cast = dynamic_cast<const LLVMPointerType *>(&other))
			return *cast->subtype == *subtype;

		return false;
	}
}
