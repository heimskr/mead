#include "mead/error/TypeError.h"
#include "mead/node/Expression.h"
#include "mead/node/Identifier.h"
#include "mead/node/TypeNode.h"
#include "mead/Compiler.h"
#include "mead/Function.h"
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
				case NodeType::FunctionDeclaration:
				case NodeType::FunctionDefinition:
					result = compileFunction(node);
					break;
				default:
					// throw std::runtime_error("Unhandled node: " + std::string(nodeTypes.at(node->type)));
					// node->debug();
					;
			}

			if (!result)
				return result;

			out << result.value() << '\n';
		}

		return out.str();
	}

	CompilerResult Compiler::compileGlobalVariable(const ASTNodePtr &node) {
		// node->debug(INFO("Global variable:\n"), 6) << '\n';

		const bool is_declaration = node->type == NodeType::VariableDeclaration;
		const bool is_definition = node->type == NodeType::VariableDefinition;

		assert(is_declaration || is_definition);

		ASTNodePtr declaration_node = is_declaration? node : node->front();
		auto declaration_id = std::dynamic_pointer_cast<Identifier>(declaration_node->front());
		assert(declaration_id);


		auto scope = program->getGlobalScope();
		auto ns = program->getGlobalNamespace();

		if (is_definition) {
			// INFO("Expr type: {}\n", getType(*scope, node->at(1)));
		}

		const std::string &identifier = declaration_id->getIdentifier();

		auto type_node = std::dynamic_pointer_cast<TypeNode>(declaration_node->at(1));
		assert(type_node);

		TypePtr stated_type = type_node->getType(ns);

		if (is_definition) {
			TypePtr expr_type = getType(*scope, node->at(1));
			if (expr_type && !expr_type->isExactlyEquivalent(*stated_type)) {
				// TODO: coercion
				throw TypeError(std::move(expr_type), std::move(stated_type));
			}
		}

		VariablePtr new_variable = std::make_shared<Variable>(identifier, stated_type);
		bool inserted = scope->insertVariable(identifier, new_variable);
		assert(inserted);

		return std::format("[\x1b[2mglobal.\x1b[22m {}]", new_variable);
	}

	CompilerResult Compiler::compileFunction(const ASTNodePtr &node) {
		// node->debug(INFO("Function:\n"), 6) << '\n';

		ASTNodePtr prototype = node->front();

		auto identifier = std::dynamic_pointer_cast<Identifier>(prototype->front());
		assert(identifier);

		auto return_type_node = std::dynamic_pointer_cast<TypeNode>(prototype->at(1));
		assert(return_type_node);

		NamespacePtr ns = program->getGlobalNamespace();
		TypePtr return_type = return_type_node->getType(ns);
		std::vector<TypePtr> argument_types;


		for (size_t i = 2; i < prototype->size(); ++i) {
			auto argument_type_node = std::dynamic_pointer_cast<TypeNode>(prototype->at(i)->at(1));
			assert(argument_type_node);
			argument_types.push_back(argument_type_node->getType(ns));
		}

		std::string name = identifier->getIdentifier();
		auto function = std::make_shared<Function>(name, std::move(return_type), std::move(argument_types));
		bool inserted = ns->insertFunction(name, function);
		assert(inserted);

		return std::format("[\x1b[2mfunction.\x1b[22m {}]", function);
	}

	TypePtr Compiler::getType(Scope &scope, const ASTNodePtr &node) {
		auto expression = std::dynamic_pointer_cast<Expression>(node);
		assert(expression);
		return expression->getType(scope);
	}
}
