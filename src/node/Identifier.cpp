#include "mead/error/ResolutionError.h"
#include "mead/node/Identifier.h"
#include "mead/Logging.h"
#include "mead/Scope.h"
#include "mead/Type.h"
#include "mead/Variable.h"

#include <cassert>

namespace mead {
	Identifier::Identifier(Token token):
		Expression(NodeType::Identifier, std::move(token)) {}

	TypePtr Identifier::getType(const Scope &scope) const {
		if (VariablePtr variable = scope.getVariable(token.value))
			return LReferenceType::wrap(variable->getType());

		throw ResolutionError(token.value);
	}
}
