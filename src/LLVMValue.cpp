#include "mead/LLVMValue.h"
#include "mead/Util.h"

#include <format>

namespace mead {
	LLVMValue::operator std::string() const {
		return std::format("{}", *this);
	}

	LLVMTypePtr LLVMIntValue::getType() const {
		return type;
	}

	std::format_context::iterator LLVMIntValue::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{} {}", static_cast<LLVMType &>(*type), value);
	}

	LLVMArrayValue::LLVMArrayValue(std::vector<LLVMValuePtr> values, LLVMTypePtr type):
	values(std::move(values)), type(std::move(type)) {
		auto *array_type = dynamic_cast<const LLVMArrayType *>(this->type.get());
		if (!array_type)
			throw std::invalid_argument("LLVMArrayValue type isn't an LLVMArrayType");

		for (const LLVMValuePtr &value : this->values) {
			if (*value->getType() != *array_type->subtype)
				throw std::invalid_argument("Array subvalue type doesn't match subtype");
		}
	}

	LLVMTypePtr LLVMArrayValue::getType() const {
		return type;
	}

	std::format_context::iterator LLVMArrayValue::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{} [{}]", type, join(values));
	}

	LLVMStructValue::LLVMStructValue(std::vector<LLVMValuePtr> values, LLVMTypePtr type):
		values(std::move(values)), type(std::move(type)) {}

	LLVMStructValue::LLVMStructValue(std::vector<LLVMValuePtr> values):
	values(std::move(values)) {
		std::vector<LLVMTypeWeakPtr> subtypes;
		subtypes.reserve(this->values.size());
		for (const LLVMValuePtr &value : this->values)
			subtypes.push_back(value->getType());
		type = std::make_shared<LLVMStructType>(std::move(subtypes));
	}

	LLVMTypePtr LLVMStructValue::getType() const {
		return type;
	}

	std::format_context::iterator LLVMStructValue::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{} {{{}}}", type, join(values));
	}

	LLVMGlobalValue::LLVMGlobalValue(std::string name, LLVMTypePtr type):
		name(std::move(name)), type(std::move(type)) {}

	LLVMTypePtr LLVMGlobalValue::getType() const {
		return type;
	}

	std::format_context::iterator LLVMGlobalValue::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{} @{}", type, name);
	}

	LLVMTypePtr LLVMNullValue::getType() const {
		return std::make_shared<LLVMPointerType>(nullptr);
	}

	std::format_context::iterator LLVMNullValue::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "ptr null");
	}
}
