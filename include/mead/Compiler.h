#pragma once

#include <expected>
#include <utility>

#include "mead/ASTNode.h"
#include "mead/Program.h"
#include "mead/Type.h"

namespace mead {
	class BasicBlock;
	class Function;
	class Scope;
	class TypeNode;

	using CompilerError = std::pair<std::string, ASTNodePtr>;
	using CompilerResult = std::expected<std::string, CompilerError>;

	class Compiler {
		public:
			Compiler();

			CompilerResult compile(std::span<const ASTNodePtr>);

		private:
			ProgramPtr program;

			CompilerResult compileGlobalVariable(const ASTNodePtr &);
			CompilerResult compileFunction(const ASTNodePtr &);

			TypePtr getType(Scope &, const ASTNodePtr &expression);
			TypePtr getType(Scope &, const TypeNode &);
	};
}
