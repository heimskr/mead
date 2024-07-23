#include "mead/node/Number.h"
#include "mead/Logging.h"
#include "mead/Type.h"
#include "mead/Util.h"

#include <cassert>

namespace mead {
	Number::Number(Token token):
		Expression(NodeType::Number, std::move(token)) {}

	TypePtr Number::getType(const Scope &scope) const {
		// TODO: allow more types
		return std::make_shared<IntType>(64, true, true);
	}

	bool Number::isConstant(const Scope &) const {
		return true;
	}
}
