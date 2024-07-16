#include "mead/Compiler.h"
#include "mead/node/Expression.h"
#include "mead/node/Identifier.h"
#include "mead/node/TypeNode.h"
#include "mead/Logging.h"
#include "mead/Namespace.h"
#include "mead/Scope.h"

#include <cassert>
#include <sstream>

namespace mead {
	Compiler::Compiler(): program(std::make_shared<Program>()) {
		program->init();
	}

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

		ASTNodePtr declaration_node = is_declaration? node : node->front();
		node->debug(INFO("Global variable:\n"), 6) << '\n';
		auto declaration_id = std::dynamic_pointer_cast<Identifier>(declaration_node->front());
		assert(declaration_id);


		auto scope = program->getGlobalScope();
		auto ns = program->getGlobalNamespace();

		if (is_definition) {
			INFO("Expr type: {}\n", getType(*scope, node->at(1)));
		}

		const std::string &identifier = declaration_id->getIdentifier();

		auto type_node = std::dynamic_pointer_cast<TypeNode>(declaration_node->at(1));
		assert(type_node);

		auto new_variable = std::make_shared<Variable>(identifier, type_node->getType(ns));
		bool inserted = scope->insertVariable(identifier, new_variable);
		assert(inserted);

		for (const auto &[name, var] : scope->getVariables()) {
			INFO("[name={}, var={}]", name, var);
		}

		return std::format("[\x1b[2mglobal var.\x1b[22m {}]", new_variable);
	}

	TypePtr Compiler::getType(Scope &scope, const ASTNodePtr &node) {
		auto expression = std::dynamic_pointer_cast<Expression>(node);
		assert(expression);
		return expression->getType(scope);
	}
}
