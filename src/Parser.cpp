#include "mead/Parser.h"
#include "mead/QualifiedType.h"
#include "mead/Util.h"

#include <cassert>
#include <expected>
#include <print>
#include <set>

namespace {
	using enum mead::TokenType;
	using enum mead::Associativity;

	const std::map<mead::TokenType, int> precedenceMap{
		{Star,                  11},
		{Slash,                 11},
		{Percent,               11},
		{Plus,                  10},
		{Minus,                 10},
		{LeftShift,              9},
		{RightShift,             9},
		{Spaceship,              8},
		{OpeningAngle,           7},
		{Leq,                    7},
		{ClosingAngle,           7},
		{Geq,                    7},
		{DoubleEquals,           6},
		{NotEqual,               6},
		{Ampersand,              5},
		{Xor,                    4},
		{Pipe,                   3},
		{DoubleAmpersand,        2},
		{DoublePipe,             1},
		{Equals,                 0},
		{PlusAssign,             0},
		{MinusAssign,            0},
		{StarAssign,             0},
		{SlashAssign,            0},
		{PercentAssign,          0},
		{LeftShiftAssign,        0},
		{RightShiftAssign,       0},
		{AmpersandAssign,        0},
		{XorAssign,              0},
		{PipeAssign,             0},
		{DoubleAmpersandAssign,  0},
		{DoublePipeAssign,       0},
	};

	const std::map<mead::TokenType, mead::Associativity> associativityMap{
		{Star,                  LeftToRight},
		{Slash,                 LeftToRight},
		{Percent,               LeftToRight},
		{Plus,                  LeftToRight},
		{Minus,                 LeftToRight},
		{LeftShift,             LeftToRight},
		{RightShift,            LeftToRight},
		{Spaceship,             LeftToRight},
		{OpeningAngle,          LeftToRight},
		{Leq,                   LeftToRight},
		{ClosingAngle,          LeftToRight},
		{Geq,                   LeftToRight},
		{DoubleEquals,          LeftToRight},
		{NotEqual,              LeftToRight},
		{Ampersand,             LeftToRight},
		{Xor,                   LeftToRight},
		{Pipe,                  LeftToRight},
		{DoubleAmpersand,       LeftToRight},
		{DoublePipe,            LeftToRight},
		{Equals,                RightToLeft},
		{PlusAssign,            RightToLeft},
		{MinusAssign,           RightToLeft},
		{StarAssign,            RightToLeft},
		{SlashAssign,           RightToLeft},
		{PercentAssign,         RightToLeft},
		{LeftShiftAssign,       RightToLeft},
		{RightShiftAssign,      RightToLeft},
		{AmpersandAssign,       RightToLeft},
		{XorAssign,             RightToLeft},
		{PipeAssign,            RightToLeft},
		{DoubleAmpersandAssign, RightToLeft},
		{DoublePipeAssign,      RightToLeft},
	};
}

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

	const Token * Parser::peek(std::span<const Token> tokens) {
		return tokens.empty()? nullptr : &tokens.front();
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
		return log.success(node);
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
		return log.success(node);
	}

	ParseResult Parser::takeIdentifier(std::span<const Token> &tokens) {
		auto log = logger("takeIdentifier");

		const Token *identifier = take(tokens, TokenType::Identifier);

		if (!identifier) {
			return log.fail("No identifier", tokens);
		}

		return log.success(ASTNode::make(NodeType::Identifier, *identifier));
	}

	ParseResult Parser::takeNumber(std::span<const Token> &tokens) {
		auto log = logger("takeNumber");

		const Token *number = take(tokens, TokenType::IntegerLiteral);

		if (!number) {
			number = take(tokens, TokenType::FloatingLiteral);
			if (!number) {
				return log.fail("No number", tokens);
			}
		}

		return log.success(ASTNode::make(NodeType::Number, *number));
	}

	ParseResult Parser::takeString(std::span<const Token> &tokens) {
		auto log = logger("takeString");

		const Token *string = take(tokens, TokenType::StringLiteral);

		if (!string) {
			return log.fail("No string", tokens);
		}

		return log.success(ASTNode::make(NodeType::String, *string));
	}

	ParseResult Parser::takeIdentifierExpression(std::span<const Token> &tokens) {
		auto log = logger("takeIdentifierExpression");
		Saver saver{tokens};

		ParseResult identifier = takeIdentifier(tokens);

		if (!identifier) {
			return log.fail("No identifier", tokens, identifier);
		}

		ParseResult prime = takePrime(tokens, *identifier);
		assert(prime);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takeNumberExpression(std::span<const Token> &tokens) {
		auto log = logger("takeNumberExpression");
		Saver saver{tokens};

		ParseResult number = takeNumber(tokens);

		if (!number) {
			return log.fail("No number", tokens, number);
		}

		ParseResult prime = takePrime(tokens, *number);
		assert(prime);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takeStringExpression(std::span<const Token> &tokens) {
		auto log = logger("takeStringExpression");
		Saver saver{tokens};

		ParseResult string = takeString(tokens);

		if (!string) {
			return log.fail("No string", tokens, string);
		}

		ParseResult prime = takePrime(tokens, *string);
		assert(prime);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takeParenthetical(std::span<const Token> &tokens) {
		auto log = logger("takeParenthetical");

		if (tokens.empty()) {
			return log.fail("No tokens", tokens);
		}

		Saver saver{tokens};

		if (!take(tokens, TokenType::OpeningParen)) {
			return log.fail("No '('", tokens);
		}

		ParseResult expr = takeExpression(tokens);

		if (!expr) {
			return log.fail("No expression", tokens, expr);
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			return log.fail("No ')'", tokens);
		}

		saver.cancel();
		return log.success(expr);
	}

	ParseResult Parser::takeParentheticalExpression(std::span<const Token> &tokens) {
		auto log = logger("takeParentheticalExpression");

		if (tokens.empty()) {
			return log.fail("No tokens", tokens);
		}

		Saver saver{tokens};

		ParseResult expr = takeParenthetical(tokens);

		if (expr) {
			ParseResult prime = takePrime(tokens, *expr);
			assert(prime);

			saver.cancel();
			return log.success(prime);
		}

		return log.fail("No parenthetical", tokens, expr);
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
		return log.success(node);
	}

	ParseResult Parser::takeBlock(std::span<const Token> &tokens) {
		auto log = logger("takeBlock");
		Saver saver{tokens};

		if (!take(tokens, TokenType::OpeningBrace)) {
			return log.fail("No '{'", tokens);
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
		return log.success(node);
	}

	ParseResult Parser::takeStatement(std::span<const Token> &tokens) {
		auto log = logger("takeStatement");

		if (ParseResult node = takeVariableDeclaration(tokens)) {
			return log.success(node);
		}

		if (ParseResult node = takeBlock(tokens)) {
			return log.success(node);
		}

		if (ParseResult node = takeExpression(tokens)) {
			if (!take(tokens, TokenType::Semicolon)) {
				return log.fail("Expression statement is missing a semicolon", tokens);
			}

			return log.success(node);
		}

		if (const Token *semicolon = take(tokens, TokenType::Semicolon)) {
			return ASTNode::make(NodeType::EmptyStatement, *semicolon);
		}

		return log.fail("No statement", tokens);
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
		return log.success(node);
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
		return log.success(node);
	}

	ParseResult Parser::takeExpression(std::span<const Token> &tokens) {
		auto log = logger("takeExpression");

		if (ParseResult expr = takeConstructorExpression(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takePrefixExpression(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeUnaryPrefixExpression(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeCastExpression(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeSizeExpression(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeNewExpression(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeIdentifierExpression(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeNumberExpression(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeStringExpression(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeParentheticalExpression(tokens)) {
			return log.success(expr);
		}

		return log.fail("No expression", tokens);
	}

	ParseResult Parser::takePrime(std::span<const Token> &tokens, const ASTNodePtr &lhs, bool exclude_binary) {
		auto log = logger("takePrime");

		if (ParseResult prime = takeScopePrime(tokens, lhs)) {
			return log.success(prime);
		}

		if (ParseResult prime = takePostfixPrime(tokens, lhs)) {
			return log.success(prime);
		}

		if (ParseResult prime = takeArgumentsPrime(tokens, lhs)) {
			return log.success(prime);
		}

		if (ParseResult prime = takeSubscriptPrime(tokens, lhs)) {
			return log.success(prime);
		}

		if (!exclude_binary) {
			if (ParseResult prime = takeBinaryPrime(tokens, lhs)) {
				return log.success(prime);
			}
		}

		return log.success(lhs);
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
		return log.success(out);
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

		ASTNodePtr node = ASTNode::make(NodeType::PrefixExpression, *oper);
		(*subexpr)->reparent(node);

		ParseResult prime = takePrime(tokens, node);
		assert(prime);

		saver.cancel();
		return log.success(prime);
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

		ASTNodePtr node = ASTNode::make(NodeType::UnaryExpression, *oper);
		(*expr)->reparent(node);

		ParseResult prime = takePrime(tokens, node);
		assert(prime);

		saver.cancel();
		return log.success(prime);
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

		ASTNodePtr lhs = ASTNode::make(NodeType::CastExpression, *cast);
		(*type)->reparent(lhs);
		(*expr)->reparent(lhs);

		ParseResult prime = takePrime(tokens, lhs);
		assert(prime);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takeScopePrime(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takeScopePrime");
		Saver saver{tokens};

		const Token *scope = take(tokens, TokenType::DoubleColon);

		if (!scope) {
			return log.fail("No '::'", tokens);
		}

		ParseResult identifier = takeIdentifier(tokens);

		if (!identifier) {
			return log.fail("No identifier", tokens, identifier);
		}

		ASTNodePtr node = ASTNode::make(NodeType::Scope, *scope);
		lhs->reparent(node);
		(*identifier)->reparent(node);

		ParseResult prime = takePrime(tokens, node);
		assert(prime);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takePostfixPrime(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePostfixPrime");
		Saver saver{tokens};

		const Token *oper = take(tokens, TokenType::DoublePlus);

		if (!oper) {
			oper = take(tokens, TokenType::DoubleMinus);
			if (!oper) {
				return log.fail("No operator", tokens);
			}
		}

		ASTNodePtr node = ASTNode::make(NodeType::Postfix, *oper);
		lhs->reparent(node);

		ParseResult prime = takePrime(tokens, node);
		assert(prime);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takeArgumentsPrime(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takeArgumentsPrime");
		Saver saver{tokens};

		const Token *opening = take(tokens, TokenType::OpeningParen);

		if (!opening) {
			return log.fail("No '('", tokens);
		}

		ParseResult arguments = takeArgumentList(tokens);

		if (!arguments) {
			return log.fail("No arguments", tokens, arguments);
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			return log.fail("No ')'", tokens);
		}

		ASTNodePtr node = ASTNode::make(NodeType::Arguments, *opening);
		lhs->reparent(node);
		(*arguments)->reparent(node);

		ParseResult prime = takePrime(tokens, node);
		assert(prime);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takeSubscriptPrime(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takeSubscriptPrime");
		Saver saver{tokens};

		const Token *opening = take(tokens, TokenType::OpeningSquare);

		if (!opening) {
			return log.fail("No '['", tokens);
		}

		ParseResult subexpr = takeExpression(tokens);

		if (!subexpr) {
			return log.fail("No subexpression", tokens, subexpr);
		}

		if (!take(tokens, TokenType::ClosingSquare)) {
			return log.fail("No ']'", tokens);
		}

		ASTNodePtr node = ASTNode::make(NodeType::Subscript, *opening);
		lhs->reparent(node);
		(*subexpr)->reparent(node);

		ParseResult prime = takePrime(tokens, node);
		assert(prime);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takePrimary(std::span<const Token> &tokens) {
		auto log = logger("takePrimary");

		if (tokens.empty()) {
			return log.fail("No tokens", Token{});
		}

		if (ParseResult expr = takeIdentifier(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeNumber(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeString(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeParenthetical(tokens)) {
			return log.success(expr);
		}

		return log.fail("No primary expression", tokens);
	}

	ParseResult Parser::takeBinaryPrime(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takeBinaryPrime");

		if (tokens.empty()) {
			return log.fail("No tokens", Token{});
		}

		Saver saver{tokens};

		ASTNodePtr node;

		if (ParseResult binary_result = takeBinary(tokens, lhs)) {
			node = *binary_result;
			std::println("\x1b[32mBinary succeeded:\n{}\x1b[39m", *node);
		} else {
			std::println("\x1b[31mBinary failed: {} {}\x1b[39m", binary_result.error().first, binary_result.error().second);
			const Token *token = &tokens.front();

			if (!precedenceMap.contains(token->type)) {
				return log.fail("Invalid token", tokens);
			}

			tokens = tokens.subspan(1);

			ParseResult rhs = takeExpression(tokens);

			if (!rhs) {
				return log.fail("No rhs", tokens, rhs);
			}

			node = ASTNode::make(NodeType::Binary, *token);
			lhs->reparent(node);
			(*rhs)->reparent(node);
		}

		ParseResult prime = takePrime(tokens, node, true);
		assert(prime);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takeBinary(std::span<const Token> &tokens, ASTNodePtr lhs, int min_precedence) {
		auto log = logger("takeBinary");

		if (tokens.empty()) {
			return log.fail("No tokens", Token{});
		}

		Saver saver{tokens};

		auto lookup = [&](const Token *token) -> std::expected<std::pair<int, Associativity>, ParseError> {
			if (!token)
				return log.fail("No token", tokens);
			auto iter = precedenceMap.find(token->type);
			if (iter == precedenceMap.end())
				return log.fail("Invalid token", tokens);
			return std::make_pair(iter->second, associativityMap.at(token->type));
		};

		const Token *lookahead = peek(tokens);

		std::println("Starting at {} with min {}", lookahead, min_precedence);

		int climbs = 0;

		for (;;) {
			if (const auto pair = lookup(lookahead); !pair || pair->first < min_precedence) {
				std::println("Lookahead lookup failed at {} for {}", __LINE__, lookahead);
				break;
			}

			const Token *op = lookahead;
			tokens = tokens.subspan(1);

			const auto op_pair = lookup(op);
			if (!op_pair) {
				std::println("Op lookup failed at {} for {}", __LINE__, op);
				// return std::unexpected(op_pair.error());
				break;
			}

			ParseResult rhs = takePrimary(tokens);
			if (!rhs) {
				std::println("Primary RHS failed at {}", __LINE__);
				// return log.fail("No rhs", tokens, rhs);
				break;
			}

			lookahead = peek(tokens);
			if (!lookahead) {
				std::println("Lookahead failed at {}", __LINE__);
				break;
			}

			const auto [op_precedence, op_associativity] = *op_pair;

			for (;;) {
				const auto lookahead_pair = lookup(lookahead);
				if (!lookahead_pair) {
					std::println("Lookahead lookup failed at {} for {}", __LINE__, lookahead);
					// return std::unexpected(lookahead_pair.error());
					break;
					// goto done;
				}

				const auto [lookahead_precedence, lookahead_associativity] = *lookahead_pair;
				const bool greater = lookahead_precedence > op_precedence;

				if (!greater && !(lookahead_associativity == Associativity::RightToLeft && lookahead_precedence == op_precedence)) {
					std::println("Precedence requirement failed");
					break;
				}

				std::println("Precedence requirement passed");

				auto binary_result = takeBinary(tokens, *rhs, greater? op_precedence + 1 : op_precedence);
				if (!binary_result) {
					// return log.fail("No binary", tokens, binary_result);
					std::println("Binary failed at {}", __LINE__);
					break;
					// goto done;
				}

				rhs = *binary_result;
				lookahead = peek(tokens);
			}

			ASTNodePtr new_lhs = ASTNode::make(NodeType::ClimbedBinary, *op);
			lhs->reparent(new_lhs);
			(*rhs)->reparent(new_lhs);
			++climbs;

			lhs = new_lhs;
		}

		// done:
		std::println("Climbs: {}, min: {}", climbs, min_precedence);
		saver.cancel();
		return lhs;
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

		ASTNodePtr node = ASTNode::make(NodeType::SizeExpression, *token);
		(*expr)->reparent(node);

		ParseResult prime = takePrime(tokens, node);
		assert(prime);

		saver.cancel();
		return log.success(prime);
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

			ASTNodePtr node = ASTNode::make(NodeType::ArrayNewExpression, *token);
			(*type)->reparent(node);
			(*expr)->reparent(node);

			ParseResult prime = takePrime(tokens, node);
			assert(prime);

			saver.cancel();
			return log.success(prime);
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

			ASTNodePtr node = ASTNode::make(NodeType::SingleNewExpression, *token);
			(*type)->reparent(node);
			(*list)->reparent(node);

			ParseResult prime = takePrime(tokens, node);
			assert(prime);

			saver.cancel();
			return log.success(prime);
		}

		log("Short new expression");

		ASTNodePtr node = ASTNode::make(NodeType::SingleNewExpression, *token);
		(*type)->reparent(node);

		ParseResult prime = takePrime(tokens, node);
		assert(prime);

		saver.cancel();
		return log.success(prime);
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
