#include "mead/Parser.h"
#include "mead/QualifiedType.h"
#include "mead/Util.h"

#include <cassert>
#include <print>

namespace mead {
	Parser::Parser() = default;

	std::optional<Token> Parser::parse(std::span<const Token> tokens) {
		auto log = logger("parse");

		int item = 0;

		for (;;) {
			log("Item {}", ++item);
			if (tokens.empty()) {
				return std::nullopt;
			}

			if (ParseResult result = takeVariableDeclaration(tokens)) {
				log("Adding variable declaration @ {}", (*result)->location());
				add(*result);
			// } else if (ParseResult result = takeVariableDefinition(tokens)) {
			// 	add(*result);
			} else if (ParseResult result = takeFunctionDeclaration(tokens)) {
				log("Adding function declaration @ {}", (*result)->location());
				add(*result);
			} else if (ParseResult result = takeFunctionDefinition(tokens)) {
				log("Adding function definition @ {}", (*result)->location());
				add(*result);
			} else if (const Token *semicolon = take(tokens, TokenType::Semicolon)) {
				log("Skipping semicolon @ {}", semicolon->location);
				;
			} else {
				log("Giving up at {}", tokens.front().location);
				return tokens.front();
			}
		}
	}

	ASTNodePtr Parser::add(ASTNodePtr node) {
		astNodes.push_back(node);
		return node;
	}

	const Token * Parser::peek(std::span<const Token> tokens, TokenType token_type) {
		if (tokens.empty()) {
			return nullptr;
		}

		if (tokens.front().type != token_type) {
			return nullptr;
		}

		return &tokens.front();
	}

	const Token * Parser::take(std::span<const Token> &tokens, TokenType token_type) {
		if (!peek(tokens, token_type)) {
			return nullptr;
		}

		const Token *token = &tokens.front();
		tokens = tokens.subspan(1);
		return token;
	}

	ParseResult Parser::takeFunctionPrototype(std::span<const Token> &tokens) {
		auto log = logger("takeFunctionPrototype");

		if (tokens.empty()) {
			log("No tokens");
			return nullptr;
		}

		Saver saver{tokens};

		ASTNodePtr node = ASTNode::make(NodeType::FunctionPrototype, tokens.front());

		if (!take(tokens, TokenType::Fn)) {
			return log.fail("No 'fn'", tokens);
		}

		ParseResult name = takeIdentifier(tokens);

		if (!name) {
			return log.fail("No '('", tokens, name);
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			return log.fail("No '('", tokens);
		}

		(*name)->reparent(node);

		std::vector<ASTNodePtr> variables;

		if (!take(tokens, TokenType::ClosingParen)) {
			do {
				ParseResult variable = takeTypedVariable(tokens);

				if (!variable) {
					return log.fail("No variable", tokens, variable);
				}

				variables.push_back(std::move(variable.value()));
			} while (take(tokens, TokenType::Comma));

			if (!take(tokens, TokenType::ClosingParen)) {
				log("No ')' @ {}", tokens.front());
				return nullptr;
			}
		}

		if (take(tokens, TokenType::Arrow)) {
			if (ParseResult return_type = takeType(tokens, nullptr)) {
				(*return_type)->reparent(node);
			} else {
				return log.fail("No return type", tokens, return_type);
			}
		} else {
			node->add(NodeType::Type, Token(TokenType::Void, "void", {}));
		}

		for (const ASTNodePtr &variable : variables) {
			variable->reparent(node);
		}

		saver.cancel();
		return node;
	}

	ParseResult Parser::takeFunctionDeclaration(std::span<const Token> &tokens) {
		auto log = logger("takeFunctionDeclaration");
		Saver saver{tokens};

		ParseResult prototype = takeFunctionPrototype(tokens);

		if (!prototype) {
			return log.fail("No prototype", tokens, prototype);
		}

		if (!take(tokens, TokenType::Semicolon)) {
			return log.fail("No ';'", tokens);
		}

		ASTNodePtr node = ASTNode::make(NodeType::FunctionDeclaration, (*prototype)->token);
		(*prototype)->reparent(node);

		saver.cancel();
		return prototype;
	}

	ParseResult Parser::takeFunctionDefinition(std::span<const Token> &tokens) {
		auto log = logger("takeFunctionDefinition");
		Saver saver{tokens};

		ParseResult prototype = takeFunctionPrototype(tokens);

		if (!prototype) {
			return log.fail("No prototype", tokens, prototype);
		}

		ParseResult block = takeBlock(tokens);

		if (!block) {
			return log.fail("No block", tokens, block);
		}

		ASTNodePtr node = ASTNode::make(NodeType::FunctionDefinition, (*prototype)->token);
		(*prototype)->reparent(node);
		(*block)->reparent(node);

		saver.cancel();
		return node;
	}

	ParseResult Parser::takeIdentifier(std::span<const Token> &tokens) {
		auto log = logger("takeIdentifier");

		const Token *identifier = take(tokens, TokenType::Identifier);

		if (!identifier) {
			return log.fail("No identifier", tokens);
		}

		return ASTNode::make(NodeType::Identifier, *identifier);
	}

	ParseResult Parser::takeTypedVariable(std::span<const Token> &tokens) {
		auto log = logger("takeTypedVariable");

		if (tokens.empty()) {
			return log.fail("No tokens", tokens);
		}

		Saver saver{tokens};

		ParseResult name = takeIdentifier(tokens);

		if (!name) {
			return log.fail("No name", tokens, name);
		}

		if (!take(tokens, TokenType::Colon)) {
			return log.fail("No ':'", tokens);
		}

		ParseResult type = takeType(tokens, nullptr);

		if (!type) {
			return log.fail("No type", tokens, type);
		}

		ASTNodePtr node = ASTNode::make(NodeType::VariableDeclaration, saver->front());
		(*name)->reparent(node);
		(*type)->reparent(node);

		saver.cancel();
		return node;
	}

	ParseResult Parser::takeBlock(std::span<const Token> &tokens) {
		auto log = logger("takeBlock");
		Saver saver{tokens};

		if (!take(tokens, TokenType::OpeningBrace)) {
			return log.fail("No '['", tokens);
		}

		ASTNodePtr node = ASTNode::make(NodeType::Block, saver->front());

		while (!take(tokens, TokenType::ClosingBrace)) {
			ParseResult statement = takeStatement(tokens);

			if (!statement) {
				return log.fail("No statement", tokens, statement);
			}

			(*statement)->reparent(node);
		}

		saver.cancel();
		return node;
	}

	ParseResult Parser::takeStatement(std::span<const Token> &tokens) {
		auto log = logger("takeStatement");

		if (ParseResult node = takeVariableDeclaration(tokens)) {
			return node;
		}

		if (ParseResult node = takeBlock(tokens)) {
			return node;
		}

		if (ParseResult node = takeExpression(tokens)) {
			if (!take(tokens, TokenType::Semicolon)) {
				return log.fail("Expression statement is missing a semicolon", tokens);
			}

			return node;
		}

		if (const Token *semicolon = take(tokens, TokenType::Semicolon)) {
			return ASTNode::make(NodeType::EmptyStatement, *semicolon);
		}

		return log.fail("No statement found", tokens);
	}

	ParseResult Parser::takeType(std::span<const Token> &tokens, QualifiedType *type_out) {
		auto log = logger("takeType");

		if (tokens.empty()) {
			return log.fail("No tokens", tokens);
		}

		Saver saver{tokens};

		std::vector<ASTNodePtr> pieces;

		std::optional<NamespacedName> name;

		ASTNodePtr node;

		if (const Token *int_type = take(tokens, TokenType::IntegerType)) {
			node = ASTNode::make(NodeType::Type, *int_type);
		} else {
			do {
				if (ParseResult piece = takeIdentifier(tokens)) {
					pieces.push_back(std::move(*piece));
				} else {
					return log.fail("No identifier", tokens);
				}
			} while (take(tokens, TokenType::DoubleColon));

			assert(!pieces.empty());

			std::vector<std::string> namespaces;

			for (size_t i = 0; i < pieces.size() - 1; ++i) {
				namespaces.push_back(pieces[i]->token.value);
			}

			name.emplace(std::move(namespaces), pieces.back()->token.value);
			node = ASTNode::make(NodeType::Type, saver->at((pieces.size() - 1) * 2));
		}

		std::vector<bool> pointer_consts;
		bool is_const = false;
		bool is_reference = false;
		bool ref_found = false;

		for (;;) {
			if (const Token *token = take(tokens, TokenType::Const)) {
				node->add(NodeType::Const, *token);
				if (const Token *star = take(tokens, TokenType::Star)) {
					node->add(NodeType::Pointer, *star);
					pointer_consts.push_back(true);
				} else if (const Token *ampersand = take(tokens, TokenType::Ampersand)) {
					node->add(NodeType::Reference, *ampersand);
					is_const = true;
					is_reference = true;
					ref_found = true;
				}
			} else if (const Token *star = take(tokens, TokenType::Star)) {
				node->add(NodeType::Pointer, *star);
				pointer_consts.push_back(false);
			} else if (const Token *ampersand = take(tokens, TokenType::Ampersand)) {
				if (ref_found) {
					return log.fail("Ref already found", tokens);
				}

				node->add(NodeType::Reference, *ampersand);
				is_const = false;
				is_reference = true;
				ref_found = true;
			} else {
				break;
			}
		}

		if (type_out) {
			if (!name) {
				name.emplace(std::vector<std::string>{}, node->token.value);
			}

			TypePtr type = Type::make(std::move(name.value()));
			typeDB.insert(type);
			*type_out = QualifiedType(std::move(pointer_consts), is_const, is_reference, std::move(type));
		}

		for (const ASTNodePtr &piece : pieces) {
			piece->reparent(node);
		}

		saver.cancel();
		return node;
	}

	ParseResult Parser::takeVariableDeclaration(std::span<const Token> &tokens) {
		auto log = logger("takeVariableDeclaration");
		Saver saver{tokens};

		ParseResult node = takeTypedVariable(tokens);

		if (!node) {
			return log.fail("No typed variable", tokens, node);
		}

		if (!take(tokens, TokenType::Semicolon)) {
			return log.fail("No ';'", tokens);
		}

		saver.cancel();
		return node;
	}

	ParseResult Parser::takeExpression(std::span<const Token> &tokens) {
		auto log = logger("takeExpression");

		if (ParseResult expr = takeConstructorExpression(tokens)) {
			return expr;
		}

		if (ParseResult expr = takePrefixExpression(tokens)) {
			return expr;
		}

		if (ParseResult expr = takeUnaryPrefixExpression(tokens)) {
			return expr;
		}

		if (ParseResult expr = takeCastExpression(tokens)) {
			return expr;
		}

		if (ParseResult expr = takeSizeExpression(tokens)) {
			return expr;
		}

		if (ParseResult expr = takeNewExpression(tokens)) {
			return expr;
		}

		return log.fail("No expression found", tokens);
	}

	ParseResult Parser::takePrime(std::span<const Token> &tokens) {
		auto log = logger("takePrime");

		if (ParseResult prime = takePostfixPrime(tokens)) {
			return prime;
		}

		// return log.fail("No prime found", tokens);
		return ASTNode::make(NodeType::EmptyPrime, Token());
	}

	ParseResult Parser::takeExpressionList(std::span<const Token> &tokens) {
		return logger("takeExpressionList").fail("Not implemented", tokens);
	}

	ParseResult Parser::takeArgumentList(std::span<const Token> &tokens) {
		return takeExpressionList(tokens);
	}

	ParseResult Parser::takeConstructorExpression(std::span<const Token> &tokens) {
		auto log = logger("takeConstructorExpression");

		if (tokens.empty()) {
			return log.fail("No tokens", tokens);
		}

		const Token &anchor = tokens.front();
		Saver saver{tokens};

		ParseResult type = takeType(tokens, nullptr);

		if (!type) {
			return log.fail("No type", tokens, type);
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			return log.fail("No '('", tokens);
		}

		ParseResult list = takeExpressionList(tokens);

		if (!list) {
			return log.fail("No list", tokens, list);
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			return log.fail("No ')'", tokens);
		}

		ASTNodePtr out = ASTNode::make(NodeType::ConstructorExpression, anchor);
		(*type)->reparent(out);
		(*list)->reparent(out);

		saver.cancel();
		return out;
	}

	ParseResult Parser::takePrefixExpression(std::span<const Token> &tokens) {
		auto log = logger("takePrefixExpression");
		Saver saver{tokens};

		const Token *oper = take(tokens, TokenType::DoublePlus);

		if (!oper) {
			oper = take(tokens, TokenType::DoubleMinus);
			if (!oper) {
				return log.fail("No '++' or '--'", tokens);
			}
		}

		ParseResult subexpr = takeExpression(tokens);

		if (!subexpr) {
			return log.fail("No subexpression", tokens, subexpr);
		}

		ParseResult prime = takePrime(tokens);

		if (!prime) {
			return log.fail("No prime", tokens, prime);
		}

		ASTNodePtr out = ASTNode::make(NodeType::PrefixExpression, *oper);
		(*subexpr)->reparent(out);
		(*prime)->reparent(out);

		saver.cancel();
		return out;
	}

	ParseResult Parser::takeUnaryPrefixExpression(std::span<const Token> &tokens) {
		auto log = logger("takeUnaryPrefixExpression");
		Saver saver{tokens};

		const Token *oper = nullptr;

		if (const Token *plus = take(tokens, TokenType::Plus)) {
			oper = plus;
		} else if (const Token *minus = take(tokens, TokenType::Minus)) {
			oper = minus;
		} else if (const Token *bang = take(tokens, TokenType::Bang)) {
			oper = bang;
		} else if (const Token *tilde = take(tokens, TokenType::Tilde)) {
			oper = tilde;
		} else {
			return log.fail("No operator", tokens);
		}

		ParseResult expr = takeExpression(tokens);

		if (!expr) {
			return log.fail("No subexpression", tokens, expr);
		}

		ParseResult prime = takePrime(tokens);

		if (!prime) {
			return log.fail("No prime", tokens, prime);
		}

		ASTNodePtr out = ASTNode::make(NodeType::UnaryExpression, *oper);
		(*expr)->reparent(out);
		(*prime)->reparent(out);

		saver.cancel();
		return out;
	}

	ParseResult Parser::takeCastExpression(std::span<const Token> &tokens) {
		auto log = logger("takeCastExpression");
		Saver saver{tokens};

		const Token *cast = take(tokens, TokenType::Cast);

		if (!cast) {
			return log.fail("No cast token", tokens);
		}

		if (!take(tokens, TokenType::OpeningAngle)) {
			return log.fail("No '<'", tokens);
		}

		ParseResult type = takeType(tokens, nullptr);

		if (!type) {
			return log.fail("No type", tokens, type);
		}

		if (!take(tokens, TokenType::ClosingAngle)) {
			return log.fail("No '>'", tokens);
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			return log.fail("No '('", tokens);
		}

		ParseResult expr = takeExpression(tokens);

		if (!expr) {
			return log.fail("No subexpression", tokens, expr);
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			return log.fail("No ')'", tokens);
		}

		ParseResult prime = takePrime(tokens);

		if (!prime) {
			return log.fail("No prime", tokens, prime);
		}

		ASTNodePtr out = ASTNode::make(NodeType::CastExpression, *cast);
		(*type)->reparent(out);
		(*expr)->reparent(out);
		(*prime)->reparent(out);

		saver.cancel();
		return out;
	}

	ParseResult Parser::takePostfixPrime(std::span<const Token> &tokens) {
		auto log = logger("takePostfixPrime");
		Saver saver{tokens};

		const Token *oper = take(tokens, TokenType::DoublePlus);

		if (!oper) {
			oper = take(tokens, TokenType::DoubleMinus);
			if (!oper) {
				return log.fail("No operator", tokens);
			}
		}

		ParseResult prime = takePrime(tokens);

		if (!prime) {
			return log.fail("No prime", tokens, prime);
		}

		ASTNodePtr out = ASTNode::make(NodeType::PostfixPrime, *oper);
		(*prime)->reparent(out);

		saver.cancel();
		return out;
	}

	ParseResult Parser::takeSizeExpression(std::span<const Token> &tokens) {
		auto log = logger("takeSizeExpression");
		Saver saver{tokens};

		const Token *token = take(tokens, TokenType::Size);

		if (!token) {
			return log.fail("No '#size'", tokens);
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			return log.fail("No '('", tokens);
		}

		ParseResult expr = takeExpression(tokens);

		if (!expr) {
			return log.fail("No subexpression", tokens, expr);
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			return log.fail("No ')'", tokens);
		}

		ParseResult prime = takePrime(tokens);

		if (!prime) {
			return log.fail("No prime", tokens, prime);
		}

		ASTNodePtr out = ASTNode::make(NodeType::SizeExpression, *token);
		(*expr)->reparent(out);
		(*prime)->reparent(out);

		saver.cancel();
		return out;
	}

	ParseResult Parser::takeNewExpression(std::span<const Token> &tokens) {
		auto log = logger("takeNewExpression");
		Saver saver{tokens};

		const Token *token = take(tokens, TokenType::New);

		if (!token) {
			return log.fail("No 'new'", tokens);
		}

		ParseResult type = takeType(tokens, nullptr);

		if (!type) {
			return log.fail("No type", tokens, type);
		}

		if (take(tokens, TokenType::OpeningSquare)) {
			log("Found '['");
			ParseResult expr = takeExpression(tokens);

			if (!expr) {
				return log.fail("No subexpression", tokens, expr);
			}

			if (!take(tokens, TokenType::ClosingSquare)) {
				return log.fail("No ']'", tokens);
			}

			ParseResult prime = takePrime(tokens);

			if (!prime) {
				return log.fail("No prime", tokens, prime);
			}

			ASTNodePtr out = ASTNode::make(NodeType::ArrayNewExpression, *token);
			(*type)->reparent(out);
			(*expr)->reparent(out);
			(*prime)->reparent(out);
			saver.cancel();
			return out;
		}

		if (take(tokens, TokenType::OpeningParen)) {
			log("Found '('");
			ParseResult list = takeArgumentList(tokens);

			if (!list) {
				return log.fail("No argument list", tokens, list);
			}

			if (!take(tokens, TokenType::ClosingParen)) {
				return log.fail("No ')'", tokens);
			}

			ParseResult prime = takePrime(tokens);

			if (!prime) {
				return log.fail("No prime", tokens, prime);
			}

			ASTNodePtr out = ASTNode::make(NodeType::SingleNewExpression, *token);
			(*type)->reparent(out);
			(*prime)->reparent(out);
			(*list)->reparent(out);

			saver.cancel();
			return out;
		}

		log("Short new expression");
		ParseResult prime = takePrime(tokens);

		if (!prime) {
			return log.fail("No prime", tokens, prime);
		}

		ASTNodePtr out = ASTNode::make(NodeType::SingleNewExpression, *token);
		(*type)->reparent(out);
		(*prime)->reparent(out);

		saver.cancel();
		return out;
	}

	std::optional<std::string> Parser::takeIdentifierPure(std::span<const Token> &tokens) {
		if (tokens.empty() || !peek(tokens, TokenType::Identifier)) {
			return std::nullopt;
		}

		std::string out = tokens.front().value;
		tokens = tokens.subspan(1);
		return std::make_optional(std::move(out));
	}
}
