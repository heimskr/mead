#pragma once

#include <expected>
#include <utility>

#include "mead/ASTNode.h"

namespace mead {
	using CompilerError = std::pair<std::string, ASTNodePtr>;
	using CompilerResult = std::expected<std::string, CompilerError>;

	class Compiler {
		public:
			Compiler() = default;

			CompilerResult compile(std::span<const ASTNodePtr>);

		private:
			CompilerResult compileGlobalVariable(const ASTNode &);
			CompilerResult compileFunction(const ASTNode &);
	};
}
