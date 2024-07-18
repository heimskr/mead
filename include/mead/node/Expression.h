#pragma once

#include "mead/ASTNode.h"

#include <memory>

namespace mead {
	class Scope;
	class Type;

	class Expression: public ASTNode {
		protected:
			using ASTNode::ASTNode;

		public:
			virtual std::shared_ptr<Type> getType(const Scope &) const = 0;
			/** Returns whether the expression can be evaluated at compile time. */
			virtual bool isConstant(const Scope &) const = 0;

			// TODO: getAddress or compileAddress or some better name
	};

	using ExpressionPtr = std::shared_ptr<Expression>;
}