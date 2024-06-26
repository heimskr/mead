#include "mead/QualifiedType.h"

namespace mead {
	QualifiedType::QualifiedType() = default;

	QualifiedType::QualifiedType(std::vector<bool> pointer_consts, bool is_const, bool is_reference, TypePtr type):
		pointerConsts(std::move(pointer_consts)), isConst(is_const), isReference(is_reference), type(std::move(type)) {}

	size_t QualifiedType::pointerLevel() const {
		return pointerConsts.size();
	}
}
