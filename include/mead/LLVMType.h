#pragma once

#include <cassert>
#include <format>
#include <memory>
#include <string>
#include <vector>

namespace mead {
	class LLVMType {
		protected:
			LLVMType() = default;

		public:
			virtual ~LLVMType() = default;

			virtual operator std::string() const;
			virtual bool operator==(const LLVMType &) const = 0;
			virtual std::format_context::iterator formatTo(std::format_context &) const = 0;
	};

	using LLVMTypePtr = std::shared_ptr<LLVMType>;
	using LLVMTypeWeakPtr = std::weak_ptr<LLVMType>;

	struct LLVMIntType: LLVMType {
		int bitWidth{};

		explicit LLVMIntType(int bit_width);

		bool operator==(const LLVMType &) const override;
		std::format_context::iterator formatTo(std::format_context &) const override;
	};

	struct LLVMArrayType: LLVMType {
		int count{};
		LLVMTypePtr subtype;

		LLVMArrayType(int count, LLVMTypePtr subtype);

		bool operator==(const LLVMType &) const override;
		std::format_context::iterator formatTo(std::format_context &) const override;
	};

	struct LLVMStructType: LLVMType {
		std::vector<LLVMTypeWeakPtr> subtypes;

		explicit LLVMStructType(std::vector<LLVMTypeWeakPtr> subtypes);

		bool operator==(const LLVMType &) const override;
		std::format_context::iterator formatTo(std::format_context &) const override;
	};

	struct LLVMPointerType: LLVMType {
		LLVMTypePtr subtype;

		LLVMPointerType(LLVMTypePtr subtype);

		bool operator==(const LLVMType &) const override;
		std::format_context::iterator formatTo(std::format_context &) const override;
	};
}

template <>
struct std::formatter<mead::LLVMType> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &type, std::format_context &ctx) const {
		return type.formatTo(ctx);
	}
};

template <>
struct std::formatter<mead::LLVMTypePtr> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &ptr, std::format_context &ctx) const {
		assert(ptr);
		return std::format_to(ctx.out(), "{}", *ptr);
	}
};

template <>
struct std::formatter<mead::LLVMTypeWeakPtr> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &weak, std::format_context &ctx) const {
		if (auto ptr = weak.lock())
			return std::format_to(ctx.out(), "{}", ptr);
		return std::format_to(ctx.out(), "(expired)");
	}
};
