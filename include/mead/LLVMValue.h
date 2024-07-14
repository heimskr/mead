#pragma once

#include "mead/LLVMType.h"

#include <concepts>
#include <string>

namespace mead {
	class LLVMValue {
		protected:
			LLVMValue() = default;

		public:
			virtual ~LLVMValue() = default;

			virtual LLVMTypePtr getType() const = 0;
			/** Doesn't include preceding type. */
			virtual operator std::string() const = 0;
	};

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
			operator std::string() const override;
	};

	class LLVMArrayValue: public LLVMValue {
		private:
	};
}
