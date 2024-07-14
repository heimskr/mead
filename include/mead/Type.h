#pragma once

#include "mead/Formattable.h"
#include "mead/NamespacedName.h"

#include <memory>
#include <string>

namespace mead {
	class Type: public Formattable {
		protected:
			Type() = default;

		public:
			virtual ~Type() = default;

			virtual std::string getName() const = 0;
			virtual operator std::string() const;
	};

	class IntType: public Type {
		private:
			int bitWidth{};
			bool isSigned{};
			char getPrefix() const;

		public:
			IntType(int bit_width, bool is_signed);

			inline int getBitWidth() const { return bitWidth; }
			std::string getName() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	class VoidType: public Type {
		public:
			VoidType();

			std::string getName() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	using TypePtr = std::shared_ptr<Type>;
}
