#include "mead/node/GetAddress.h"
#include "mead/Scope.h"
#include "mead/Type.h"
#include "mead/Variable.h"

#include <cassert>

namespace mead {
	GetAddress::GetAddress(Token token):
		Expression(NodeType::GetAddress, std::move(token)) {}

	std::shared_ptr<Type> GetAddress::getType(const Scope &scope) const {
		auto subexpr = std::dynamic_pointer_cast<Expression>(front());
		assert(subexpr);
		TypePtr subtype = subexpr->getType(scope);
		assert(subtype);
		return std::make_shared<PointerType>(std::move(subtype));
	}

	bool GetAddress::isConstant(const Scope &) const {
		return false;
	}
}
