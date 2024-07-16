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
			bool isConst = false;
			Type(std::string name, bool is_const);
			const char * getConstSuffix() const;

		public:
			virtual ~Type() = default;

			virtual std::string getName() const = 0;
			virtual operator std::string() const;
			virtual LLVMTypePtr toLLVM() const = 0;
			virtual bool getConst() const;
			virtual void setConst(bool);
	};

	using TypePtr = std::shared_ptr<Type>;

	class IntType: public Type {
		private:
			int bitWidth{};
			bool isSigned{};
			char getPrefix() const;
			std::string getNameImpl() const;

		public:
			IntType(int bit_width, bool is_signed, bool is_const = false);

			inline int getBitWidth() const { return bitWidth; }
			std::string getName() const override;
			LLVMTypePtr toLLVM() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	class VoidType: public Type {
		public:
			explicit VoidType(bool is_const = false);

			std::string getName() const override;
			LLVMTypePtr toLLVM() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	class PointerType: public Type {
		private:
			TypePtr subtype;
			std::string getNameImpl() const;

		public:
			explicit PointerType(TypePtr subtype, bool is_const = false);

			std::string getName() const override;
			LLVMTypePtr toLLVM() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	class LReferenceType: public Type {
		private:
			TypePtr subtype;
			std::string getNameImpl() const;

		public:
			explicit LReferenceType(TypePtr subtype, bool is_const = false);

			std::string getName() const override;
			LLVMTypePtr toLLVM() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	class ClassType: public Type {
		private:
			std::weak_ptr<Namespace> owner;
			std::string getNameImpl() const;

		public:
			ClassType(std::string name, std::weak_ptr<Namespace> owner, bool is_const = false);

			Namespace & getNamespace() const;
			std::string getName() const override;
			LLVMTypePtr toLLVM() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};
}
