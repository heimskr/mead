#include "mead/node/Binary.h"
#include "mead/Type.h"

namespace mead {
	Binary::Binary(Token token):
		Expression(NodeType::Binary, std::move(token)) {}

	ExpressionPtr Binary::getLHS() const {
		auto lhs = std::dynamic_pointer_cast<Expression>(at(0));
		assert(lhs);
		return lhs;
	}

	ExpressionPtr Binary::getRHS() const {
		auto rhs = std::dynamic_pointer_cast<Expression>(at(1));
		assert(rhs);
		return rhs;
	}

	TypePtr Binary::getType(const Scope &scope) const {
		ExpressionPtr lhs = getLHS();
		ExpressionPtr rhs = getRHS();

		TypePtr lhs_type = lhs->getType(scope);
		assert(lhs_type);

		TypePtr rhs_type = rhs->getType(scope);
		assert(rhs_type);

		if (rhs_type->isConvertibleTo(*lhs_type))
			return lhs_type;

		if (lhs_type->isConvertibleTo(*rhs_type))
			return rhs_type;

		return std::make_shared<InvalidType>(true);
	}

	bool Binary::isConstant(const Scope &scope) const {
		return getLHS()->isConstant(scope) && getRHS()->isConstant(scope) && !std::dynamic_pointer_cast<InvalidType>(getType(scope));
	}
}
