#pragma once

#include "mead/ASTNode.h"

namespace mead {
	class Namespace;
	class Type;

	class TypeNode: public ASTNode {
		public:
			TypeNode(Token);

			std::shared_ptr<Type> getType(const std::shared_ptr<Namespace> &) const;
	};
}
