#include "mead/Compiler.h"

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
					result = compileGlobalVariable(*node);
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

	CompilerResult Compiler::compileGlobalVariable(const ASTNode &node) {
		assert(node.type == NodeType::VariableDeclaration || node.type == NodeType::VariableDefinition);
		node.debug();
		return "";
	}
}
