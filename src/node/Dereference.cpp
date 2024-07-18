#include "mead/node/Dereference.h"
#include "mead/Scope.h"
#include "mead/Type.h"
#include "mead/Variable.h"

#include <cassert>

namespace mead {
	Dereference::Dereference(Token token):
		Expression(NodeType::Deref, std::move(token)) {}

	std::shared_ptr<Type> Dereference::getType(const Scope &scope) const {
		auto subexpr = std::dynamic_pointer_cast<Expression>(front());
		assert(subexpr);
		TypePtr subtype = subexpr->getType(scope);
		assert(subtype);
		return subtype->dereference();
	}

	bool Dereference::isConstant(const Scope &) const {
		// Maybe it could be allowed for string literals...?
		return false;
	}
}
