#pragma once

#include "mead/LLVMType.h"

#include <concepts>
#include <format>
#include <memory>
#include <string>

namespace mead {
	class LLVMValue {
		protected:
			LLVMValue() = default;

		public:
			virtual ~LLVMValue() = default;

			virtual LLVMTypePtr getType() const = 0;
			/** Includes preceding type. */
			virtual operator std::string() const;
			/** Includes preceding type. */
			virtual std::format_context::iterator formatTo(std::format_context &) const = 0;
	};

	using LLVMValuePtr = std::shared_ptr<LLVMValue>;

	class LLVMIntValue: public LLVMValue {
		private:
			uint64_t value;
			std::shared_ptr<LLVMIntType> type;

		public:
			template <std::integral T>
			LLVMIntValue(T value):
				value(value),
				type(std::make_shared<LLVMIntType>(sizeof(value) * 8)) {}

			LLVMTypePtr getType() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	class LLVMArrayValue: public LLVMValue {
		private:
			std::vector<LLVMValuePtr> values;
			LLVMTypePtr type;

		public:
			LLVMArrayValue(std::vector<LLVMValuePtr> values, LLVMTypePtr type);
			LLVMTypePtr getType() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
			inline const auto & getValues() const { return values; }
	};

	class LLVMStructValue: public LLVMValue {
		private:
			std::vector<LLVMValuePtr> values;
			LLVMTypePtr type;

		public:
			LLVMStructValue(std::vector<LLVMValuePtr> values, LLVMTypePtr type);
			LLVMStructValue(std::vector<LLVMValuePtr> values);
			LLVMTypePtr getType() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
			inline const auto & getValues() const { return values; }
	};

	class LLVMGlobalValue: public LLVMValue {
		private:
			/** Doesn't include a leading @. */
			std::string name;
			LLVMTypePtr type;

		public:
			LLVMGlobalValue(std::string name, LLVMTypePtr type);
			LLVMTypePtr getType() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
			inline const auto & getName() const { return name; }
	};

	class LLVMNullValue: public LLVMValue {
		public:
			LLVMNullValue() = default;
			LLVMTypePtr getType() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};
}

template <>
struct std::formatter<mead::LLVMValue> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &value, std::format_context &ctx) const {
		return value.formatTo(ctx);
	}
};

template <>
struct std::formatter<mead::LLVMValuePtr> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &ptr, std::format_context &ctx) const {
		assert(ptr);
		return ptr->formatTo(ctx);
	}
};
