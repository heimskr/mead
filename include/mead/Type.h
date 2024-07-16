#pragma once

#include "mead/Formattable.h"
#include "mead/LLVMType.h"
#include "mead/Symbol.h"

#include <memory>
#include <string>

namespace mead {
	class Namespace;

	class Type: public Symbol, public Formattable {
		protected:
			using Symbol::Symbol;

		public:
			virtual ~Type() = default;

			virtual std::string getName() const = 0;
			virtual operator std::string() const;
			virtual LLVMTypePtr toLLVM() const = 0;
	};

	using TypePtr = std::shared_ptr<Type>;

	class IntType: public Type {
		private:
			int bitWidth{};
			bool isSigned{};
			char getPrefix() const;

		public:
			IntType(int bit_width, bool is_signed);

			inline int getBitWidth() const { return bitWidth; }
			std::string getName() const override;
			LLVMTypePtr toLLVM() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	class VoidType: public Type {
		public:
			VoidType();

			std::string getName() const override;
			LLVMTypePtr toLLVM() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	class PointerType: public Type {
		private:
			TypePtr subtype;

		public:
			PointerType(TypePtr subtype);

			std::string getName() const override;
			LLVMTypePtr toLLVM() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	class ClassType: public Type {
		private:
			std::weak_ptr<Namespace> owner;

		public:
			ClassType(std::string name, std::weak_ptr<Namespace> owner);

			Namespace & getNamespace() const;
			std::string getName() const override;
			LLVMTypePtr toLLVM() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};
}
