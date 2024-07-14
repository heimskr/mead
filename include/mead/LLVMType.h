#pragma once

#include "mead/Formattable.h"

#include <cassert>
#include <format>
#include <memory>
#include <string>
#include <vector>

namespace mead {
	class LLVMType: public Formattable {
		protected:
			LLVMType() = default;

		public:
			virtual ~LLVMType() = default;

			virtual operator std::string() const;
			virtual bool operator==(const LLVMType &) const = 0;
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

	struct LLVMVoidType: LLVMType {
		LLVMVoidType();

		bool operator==(const LLVMType &) const override;
		std::format_context::iterator formatTo(std::format_context &) const override;
	};
}
