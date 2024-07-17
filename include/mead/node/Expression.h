#pragma once

#include "mead/ASTNode.h"

namespace mead {
	class Scope;
	class Type;

	class Expression: public ASTNode {
		protected:
			using ASTNode::ASTNode;

		public:
			virtual std::shared_ptr<Type> getType(const Scope &) const = 0;

			// TODO: getAddress or compileAddress or some better name
	};
}