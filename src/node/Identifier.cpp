#include "mead/error/ResolutionError.h"
#include "mead/node/Identifier.h"
#include "mead/Logging.h"
#include "mead/Scope.h"
#include "mead/Variable.h"

#include <cassert>

namespace mead {
	Identifier::Identifier(Token token):
		Expression(NodeType::Identifier, std::move(token)) {}

	std::shared_ptr<Type> Identifier::getType(const Scope &scope) const {
		if (VariablePtr variable = scope.getVariable(token.value)) {
			INFO("Found variable for {}", token.value);
			return variable->getType();
		}

		throw ResolutionError(token.value);
	}
}
