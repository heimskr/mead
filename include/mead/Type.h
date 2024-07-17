#pragma once

#include "mead/Formattable.h"
#include "mead/LLVMType.h"
#include "mead/Symbol.h"

#include <memory>
#include <string>

namespace mead {
	class Namespace;

	class Type;
	using TypePtr = std::shared_ptr<Type>;

	class Type: public Symbol, public Formattable, public std::enable_shared_from_this<Type> {
		protected:
			bool isConst = false;
			Type(std::string name, bool is_const);
			const char * getConstSuffix() const;

		public:
			virtual ~Type() = default;

			virtual std::string getName() const = 0;
			virtual TypePtr copy() const = 0;
			virtual operator std::string() const;
			virtual LLVMTypePtr toLLVM() const = 0;
			virtual bool getConst() const;
			virtual void setConst(bool);
			virtual bool isExactlyEquivalent(const Type &) const = 0;
			virtual TypePtr unwrapLReference();
			/** Returns nullptr if the type can't be dereferenced. */
			virtual TypePtr dereference() const;
	};


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
			TypePtr copy() const override;
			bool isExactlyEquivalent(const Type &) const override;
			LLVMTypePtr toLLVM() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	class VoidType: public Type {
		public:
			explicit VoidType(bool is_const = false);

			std::string getName() const override;
			TypePtr copy() const override;
			bool isExactlyEquivalent(const Type &) const override;
			LLVMTypePtr toLLVM() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	class PointerType: public Type {
		private:
			TypePtr subtype;
			std::string getNameImpl() const;

		public:
			explicit PointerType(const TypePtr &subtype, bool is_const = false);

			std::string getName() const override;
			TypePtr copy() const override;
			bool isExactlyEquivalent(const Type &) const override;
			LLVMTypePtr toLLVM() const override;
			TypePtr dereference() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	class LReferenceType: public Type {
		private:
			TypePtr subtype;
			std::string getNameImpl() const;

		public:
			explicit LReferenceType(const TypePtr &subtype, bool is_const = false);

			std::string getName() const override;
			TypePtr copy() const override;
			bool isExactlyEquivalent(const Type &) const override;
			LLVMTypePtr toLLVM() const override;
			TypePtr unwrapLReference() override;
			std::format_context::iterator formatTo(std::format_context &) const override;

			static std::shared_ptr<LReferenceType> wrap(const TypePtr &);
	};

	class ClassType: public Type {
		private:
			std::weak_ptr<Namespace> owner;
			std::string getNameImpl() const;
			// TODO: fields

		public:
			ClassType(std::string name, std::weak_ptr<Namespace> owner, bool is_const = false);

			Namespace & getNamespace() const;
			std::string getName() const override;
			TypePtr copy() const override;
			bool isExactlyEquivalent(const Type &) const override;
			LLVMTypePtr toLLVM() const override;
			std::format_context::iterator formatTo(std::format_context &) const override;
	};
}
