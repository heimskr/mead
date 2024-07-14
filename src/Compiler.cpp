#include "mead/Compiler.h"
#include "mead/Logging.h"
#include "mead/Scope.h"

#include <cassert>
#include <sstream>

namespace mead {
	CompilerResult Compiler::compile(std::span<const ASTNodePtr> nodes) {
		std::stringstream out;

		for (const ASTNodePtr &node : nodes) {
			CompilerResult result;

			switch (node->type) {
				case NodeType::VariableDeclaration:
				case NodeType::VariableDefinition:
					result = compileGlobalVariable(node);
					break;
				default:
					// throw std::runtime_error("Unhandled node: " + std::string(nodeTypes.at(node->type)));
					;
			}

			if (!result)
				return result;

			out << result.value() << '\n';
		}

		return out.str();
	}

	CompilerResult Compiler::compileGlobalVariable(const ASTNodePtr &node) {
		const bool is_declaration = node->type == NodeType::VariableDeclaration;
		const bool is_definition = node->type == NodeType::VariableDefinition;

		assert(is_declaration || is_definition);

		ASTNodePtr declaration = is_declaration? node : node->front();

		node->debug(INFO("Global variable:\n"), 6) << '\n';

		return "";
	}

	TypePtr Compiler::getType(Scope &scope, const ASTNodePtr &expression) {
		return {};
	}
}
