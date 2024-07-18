#include "mead/Logging.h"
#include "mead/Namespace.h"
#include "mead/Type.h"

#include <cassert>

namespace mead {
	Type::Type(std::string name, bool is_const):
		Symbol(std::move(name)), isConst(is_const) {}

	const char * Type::getConstSuffix() const {
		return isConst? " const" : "";
	}

	Type::operator std::string() const {
		return std::format("{}", *this);
	}

	bool Type::getConst() const {
		return isConst;
	}

	void Type::setConst(bool value) {
		isConst = value;
	}

	bool Type::isConvertibleTo(const Type &other) const {
		if (this == &other)
			return true;

		return isExactlyEquivalent(other, true);
	}

	TypePtr Type::unwrapLReference() {
		return shared_from_this();
	}

	TypePtr Type::dereference() const {
		return nullptr;
	}

	char IntType::getPrefix() const {
		return isSigned? 'i' : 'u';
	}

	IntType::IntType(int bit_width, bool is_signed, bool is_const):
		Type(getPrefix() + std::to_string(bit_width), is_const), bitWidth(bit_width), isSigned(is_signed) {}

	std::string IntType::getNameImpl() const {
		return std::format("{}{}{}", getPrefix(), bitWidth, getConstSuffix());
	}

	std::string IntType::getName() const {
		return getNameImpl();
	}

	TypePtr IntType::copy() const {
		return std::make_shared<IntType>(*this);
	}

	bool IntType::isExactlyEquivalent(const Type &other, bool ignore_const) const {
		if (this == &other)
			return true;

		if (!ignore_const && getConst() != other.getConst())
			return false;

		if (const auto *cast = dynamic_cast<const IntType *>(&other))
			return cast->bitWidth == bitWidth && cast->isSigned == isSigned;

		return false;
	}

	LLVMTypePtr IntType::toLLVM() const {
		return std::make_shared<LLVMIntType>(bitWidth);
	}

	std::format_context::iterator IntType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{}{}{}", getPrefix(), bitWidth, getConstSuffix());
	}

	VoidType::VoidType(bool is_const):
		Type("void", is_const) {}

	std::string VoidType::getName() const {
		return "void";
	}

	TypePtr VoidType::copy() const {
		return std::make_shared<VoidType>();
	}

	bool VoidType::isExactlyEquivalent(const Type &other, bool ignore_const) const {
		return this == &other || ((ignore_const || getConst() == other.getConst()) && dynamic_cast<const VoidType *>(&other));
	}

	LLVMTypePtr VoidType::toLLVM() const {
		return std::make_shared<LLVMVoidType>();
	}

	std::format_context::iterator VoidType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "void");
	}

	PointerType::PointerType(const TypePtr &subtype, bool is_const):
		Type(getNameImpl(), is_const), subtype(subtype->unwrapLReference()) {}

	std::string PointerType::getNameImpl() const {
		return std::format("{}*{}", subtype, getConstSuffix());
	}

	std::string PointerType::getName() const {
		return getNameImpl();
	}

	TypePtr PointerType::copy() const {
		assert(subtype);
		return std::make_shared<PointerType>(subtype);
	}

	bool PointerType::isExactlyEquivalent(const Type &other, bool ignore_const) const {
		if (this == &other)
			return true;

		if (!ignore_const && getConst() != other.getConst())
			return false;

		if (const auto *cast = dynamic_cast<const PointerType *>(&other))
			return subtype->isExactlyEquivalent(*cast->subtype, false);

		return false;
	}

	bool PointerType::isConvertibleTo(const Type &other) const {
		// TODO: class shenanigans

		if (this == &other)
			return true;

		if (const auto *cast = dynamic_cast<const PointerType *>(&other))
			return (!subtype->getConst() || cast->subtype->getConst()) && subtype->isExactlyEquivalent(*cast->subtype, true);

		return false;
	}

	LLVMTypePtr PointerType::toLLVM() const {
		return std::make_shared<LLVMPointerType>(subtype->toLLVM());
	}

	TypePtr PointerType::dereference() const {
		return LReferenceType::wrap(subtype);
	}

	std::format_context::iterator PointerType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{}*{}", subtype, getConstSuffix());
	}

	LReferenceType::LReferenceType(const TypePtr &subtype, bool is_const):
		Type(getNameImpl(), is_const), subtype(subtype->unwrapLReference()) {}

	std::string LReferenceType::getNameImpl() const {
		return std::format("{}&{}", subtype, getConstSuffix());
	}

	std::string LReferenceType::getName() const {
		return getNameImpl();
	}

	TypePtr LReferenceType::copy() const {
		assert(subtype);
		return std::make_shared<LReferenceType>(subtype);
	}

	bool LReferenceType::isExactlyEquivalent(const Type &other, bool ignore_const) const {
		if (this == &other)
			return true;

		// Technically, LReferences are basically const by nature, but still...
		if (!ignore_const && getConst() != other.getConst())
			return false;

		if (const auto *cast = dynamic_cast<const LReferenceType *>(&other))
			return subtype->isExactlyEquivalent(*cast->subtype, false);

		return false;
	}

	bool LReferenceType::isConvertibleTo(const Type &other) const {
		// TODO: class shenanigans...?

		if (this == &other)
			return true;

		if (const auto *cast = dynamic_cast<const LReferenceType *>(&other))
			return (!subtype->getConst() || cast->subtype->getConst()) && subtype->isExactlyEquivalent(*cast->subtype, true);

		return false;
	}

	LLVMTypePtr LReferenceType::toLLVM() const {
		assert(subtype);
		return std::make_shared<LLVMPointerType>(subtype->toLLVM());
	}

	TypePtr LReferenceType::unwrapLReference() {
		return subtype;
	}

	std::format_context::iterator LReferenceType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{}&{}", subtype, getConstSuffix());
	}

	std::shared_ptr<LReferenceType> LReferenceType::wrap(const TypePtr &type) {
		if (auto cast = std::dynamic_pointer_cast<LReferenceType>(type))
			return cast;
		return std::make_shared<LReferenceType>(type);
	}

	ClassType::ClassType(std::string name, std::weak_ptr<Namespace> owner, bool is_const):
		Type(std::move(name), is_const), owner(std::move(owner)) {}

	Namespace & ClassType::getNamespace() const {
		auto locked = owner.lock();
		assert(locked);
		return *locked;
	}

	std::string ClassType::getNameImpl() const {
		return std::format("{}::{}{}", getNamespace().getFullName(), name, getConstSuffix());
	}

	std::string ClassType::getName() const {
		return getNameImpl();
	}

	TypePtr ClassType::copy() const {
		return std::make_shared<ClassType>(*this);
	}

	bool ClassType::isExactlyEquivalent(const Type &other, bool ignore_const) const {
		if (this == &other)
			return true;

		if (!ignore_const && getConst() != other.getConst())
			return false;

		if (const auto *cast = dynamic_cast<const ClassType *>(&other))
			return cast->name == name && cast->owner.lock() == owner.lock();

		return false;
	}

	LLVMTypePtr ClassType::toLLVM() const {
		assert(false);
		return {};
	}

	std::format_context::iterator ClassType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "class {}", getNameImpl());
	}

	InvalidType::InvalidType(bool is_const):
		Type("<error>", is_const) {}

	std::string InvalidType::getName() const {
		return "<error>";
	}

	TypePtr InvalidType::copy() const {
		return std::make_shared<InvalidType>();
	}

	bool InvalidType::isExactlyEquivalent(const Type &other, bool ignore_const) const {
		return this == &other || ((ignore_const || getConst() == other.getConst()) && dynamic_cast<const InvalidType *>(&other));
	}

	LLVMTypePtr InvalidType::toLLVM() const {
		return std::make_shared<LLVMPoisonType>();
	}

	std::format_context::iterator InvalidType::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "<error>");
	}
}
