#pragma once

#include <expected>
#include <utility>

#include "mead/ASTNode.h"
#include "mead/Type.h"

namespace mead {
	class Scope;

	using CompilerError = std::pair<std::string, ASTNodePtr>;
	using CompilerResult = std::expected<std::string, CompilerError>;

	class Compiler {
		public:
			Compiler() = default;

			CompilerResult compile(std::span<const ASTNodePtr>);

		private:
			CompilerResult compileGlobalVariable(const ASTNodePtr &);
			CompilerResult compileFunction(const ASTNodePtr &);

			TypePtr getType(Scope &, const ASTNodePtr &expression);
	};
}
