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

			if (ASTNodePtr node = takeVariableDeclaration(tokens)) {
				log("Adding variable declaration @ {}", node->location());
				add(node);
			// } else if (ASTNodePtr node = takeVariableDefinition(tokens)) {
			// 	add(node);
			} else if (ASTNodePtr node = takeFunctionDeclaration(tokens)) {
				log("Adding function declaration @ {}", node->location());
				add(node);
			} else if (ASTNodePtr node = takeFunctionDefinition(tokens)) {
				log("Adding function definition @ {}", node->location());
				add(node);
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

	ASTNodePtr Parser::takeFunctionPrototype(std::span<const Token> &tokens) {
		auto log = logger("takeFunctionPrototype");

		if (tokens.empty()) {
			log("No tokens");
			return nullptr;
		}

		Saver saver{tokens};

		ASTNodePtr node = ASTNode::make(NodeType::FunctionPrototype, tokens.front());

		if (!take(tokens, TokenType::Fn)) {
			log("No 'fn' @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr name = takeIdentifier(tokens);

		if (!name) {
			log("No name @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			log("No '(' @ {}", tokens.front());
			return nullptr;
		}

		name->reparent(node);

		std::vector<ASTNodePtr> variables;

		if (!take(tokens, TokenType::ClosingParen)) {
			do {
				ASTNodePtr variable = takeTypedVariable(tokens);

				if (!variable) {
					log("No variable @ {}", tokens.front());
					return nullptr;
				}

				variables.push_back(std::move(variable));
			} while (take(tokens, TokenType::Comma));

			if (!take(tokens, TokenType::ClosingParen)) {
				log("No ')' @ {}", tokens.front());
				return nullptr;
			}
		}

		if (take(tokens, TokenType::Arrow)) {
			if (ASTNodePtr return_type = takeType(tokens, nullptr)) {
				return_type->reparent(node);
			} else {
				log("No return type @ {}", tokens.front());
				return nullptr;
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

	ASTNodePtr Parser::takeFunctionDeclaration(std::span<const Token> &tokens) {
		auto log = logger("takeFunctionDeclaration");
		Saver saver{tokens};

		ASTNodePtr prototype = takeFunctionPrototype(tokens);

		if (!prototype) {
			log("No prototype @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::Semicolon)) {
			log("No ';' @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr node = ASTNode::make(NodeType::FunctionDeclaration, prototype->token);
		prototype->reparent(node);

		saver.cancel();
		return prototype;
	}

	ASTNodePtr Parser::takeFunctionDefinition(std::span<const Token> &tokens) {
		auto log = logger("takeFunctionDefinition");
		Saver saver{tokens};

		ASTNodePtr prototype = takeFunctionPrototype(tokens);

		if (!prototype) {
			log("No prototype @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr block = takeBlock(tokens);

		if (!block) {
			log("No block @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr node = ASTNode::make(NodeType::FunctionDefinition, prototype->token);
		prototype->reparent(node);
		block->reparent(node);

		saver.cancel();
		return node;
	}

	ASTNodePtr Parser::takeIdentifier(std::span<const Token> &tokens) {
		auto log = logger("takeIdentifier");

		if (!peek(tokens, TokenType::Identifier)) {
			log("No identifier @ {}", tokens.front());
			return nullptr;
		}

		const Token &token = tokens.front();
		tokens = tokens.subspan(1);
		return ASTNode::make(NodeType::Identifier, token);
	}

	ASTNodePtr Parser::takeTypedVariable(std::span<const Token> &tokens) {
		auto log = logger("takeTypedVariable");

		if (tokens.empty()) {
			log("No tokens @ {}", tokens.front());
			return nullptr;
		}

		Saver saver{tokens};

		ASTNodePtr name = takeIdentifier(tokens);

		if (!name) {
			log("No name @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::Colon)) {
			log("No ':' @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr type = takeType(tokens, nullptr);

		if (!type) {
			log("No type @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr node = ASTNode::make(NodeType::VariableDeclaration, saver->front());
		name->reparent(node);
		type->reparent(node);

		saver.cancel();
		return node;
	}

	ASTNodePtr Parser::takeBlock(std::span<const Token> &tokens) {
		auto log = logger("takeBlock");
		Saver saver{tokens};

		if (!take(tokens, TokenType::OpeningBrace)) {
			log("No '[' @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr node = ASTNode::make(NodeType::Block, saver->front());

		while (!take(tokens, TokenType::ClosingBrace)) {
			ASTNodePtr statement = takeStatement(tokens);

			if (!statement) {
				log("No statement @ {}", tokens.front());
				return nullptr;
			}

			statement->reparent(node);
		}

		saver.cancel();
		return node;
	}

	ASTNodePtr Parser::takeStatement(std::span<const Token> &tokens) {
		auto log = logger("takeStatement");

		if (ASTNodePtr node = takeVariableDeclaration(tokens)) {
			return node;
		}

		if (ASTNodePtr node = takeBlock(tokens)) {
			return node;
		}

		if (ASTNodePtr node = takeExpression(tokens)) {
			if (!take(tokens, TokenType::Semicolon)) {
				log("Expression statement is missing a semicolon");
				return nullptr;
			}

			return node;
		}

		if (const Token *semicolon = take(tokens, TokenType::Semicolon)) {
			return ASTNode::make(NodeType::EmptyStatement, *semicolon);
		}

		log("No statement found @ {}", tokens.front());
		return nullptr;
	}

	ASTNodePtr Parser::takeType(std::span<const Token> &tokens, QualifiedType *type_out) {
		auto log = logger("takeType");

		if (tokens.empty()) {
			log("No tokens @ {}", tokens.front());
			return nullptr;
		}

		Saver saver{tokens};

		std::vector<ASTNodePtr> pieces;

		std::optional<NamespacedName> name;

		ASTNodePtr node;

		if (const Token *int_type = take(tokens, TokenType::IntegerType)) {
			node = ASTNode::make(NodeType::Type, *int_type);
		} else {
			do {
				if (ASTNodePtr piece = takeIdentifier(tokens)) {
					pieces.push_back(std::move(piece));
				} else {
					log("No identifier @ {}", tokens.front());
					return nullptr;
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
					log("Ref not found @ {}", tokens.front());
					return nullptr;
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

	std::optional<std::string> Parser::takeIdentifierPure(std::span<const Token> &tokens) {
		if (tokens.empty() || !peek(tokens, TokenType::Identifier)) {
			return std::nullopt;
		}

		std::string out = tokens.front().value;
		tokens = tokens.subspan(1);
		return std::make_optional(std::move(out));
	}

	ASTNodePtr Parser::takeVariableDeclaration(std::span<const Token> &tokens) {
		auto log = logger("takeVariableDeclaration");
		Saver saver{tokens};

		ASTNodePtr node = takeTypedVariable(tokens);

		if (!node) {
			log("No typed variable @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::Semicolon)) {
			log("No ';' @ {}", tokens.front());
			return nullptr;
		}

		saver.cancel();
		return node;
	}

	ASTNodePtr Parser::takeExpression(std::span<const Token> &tokens) {
		auto log = logger("takeExpression");

		if (ASTNodePtr expr = takeConstructorExpression(tokens)) {
			return expr;
		}

		if (ASTNodePtr expr = takePrefixExpression(tokens)) {
			return expr;
		}

		if (ASTNodePtr expr = takeUnaryPrefixExpression(tokens)) {
			return expr;
		}

		if (ASTNodePtr expr = takeCastExpression(tokens)) {
			return expr;
		}

		if (ASTNodePtr expr = takeSizeExpression(tokens)) {
			return expr;
		}

		if (ASTNodePtr expr = takeNewExpression(tokens)) {
			return expr;
		}

		log("No expression found @ {}", tokens.front());
		return nullptr;
	}

	ASTNodePtr Parser::takePrime(std::span<const Token> &tokens) {
		auto log = logger("takePrime");

		if (ASTNodePtr prime = takePostfixPrime(tokens)) {
			return prime;
		}

		log("No prime found @ {}", tokens.front());
		return ASTNode::make(NodeType::EmptyPrime, Token());
	}

	ASTNodePtr Parser::takeExpressionList(std::span<const Token> &tokens) {
		logger("takeExpressionList")("Not implemented");
		return nullptr;
	}

	ASTNodePtr Parser::takeArgumentList(std::span<const Token> &tokens) {
		return takeExpressionList(tokens);
	}

	ASTNodePtr Parser::takeConstructorExpression(std::span<const Token> &tokens) {
		auto log = logger("takeConstructorExpression");

		if (tokens.empty()) {
			log("No tokens @ {}", tokens.front());
			return nullptr;
		}

		const Token &anchor = tokens.front();
		Saver saver{tokens};

		ASTNodePtr type = takeType(tokens, nullptr);

		if (!type) {
			log("No type @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			log("No '(' @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr list = takeExpressionList(tokens);

		if (!list) {
			log("No list @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			log("No ')' @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr out = ASTNode::make(NodeType::ConstructorExpression, anchor);
		type->reparent(out);
		list->reparent(out);

		saver.cancel();
		return out;
	}

	ASTNodePtr Parser::takePrefixExpression(std::span<const Token> &tokens) {
		auto log = logger("takePrefixExpression");
		Saver saver{tokens};

		const Token *oper = take(tokens, TokenType::DoublePlus);

		if (!oper) {
			oper = take(tokens, TokenType::DoubleMinus);
			if (!oper) {
				log("No '++' or '--' @ {}", tokens.front());
				return nullptr;
			}
		}

		ASTNodePtr subexpr = takeExpression(tokens);

		if (!subexpr) {
			log("No subexpression @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr prime = takePrime(tokens);

		if (!prime) {
			log("No prime @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr out = ASTNode::make(NodeType::PrefixExpression, *oper);
		subexpr->reparent(out);
		prime->reparent(out);

		saver.cancel();
		return out;
	}

	ASTNodePtr Parser::takeUnaryPrefixExpression(std::span<const Token> &tokens) {
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
			log("No operator @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr expr = takeExpression(tokens);

		if (!expr) {
			log("No subexpression @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr prime = takePrime(tokens);

		if (!prime) {
			log("No prime @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr out = ASTNode::make(NodeType::UnaryExpression, *oper);
		expr->reparent(out);
		prime->reparent(out);

		saver.cancel();
		return out;
	}

	ASTNodePtr Parser::takeCastExpression(std::span<const Token> &tokens) {
		auto log = logger("takeCastExpression");
		Saver saver{tokens};

		const Token *cast = take(tokens, TokenType::Cast);

		if (!cast) {
			log("No cast token @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::OpeningAngle)) {
			log("No '<' @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr type = takeType(tokens, nullptr);

		if (!type) {
			log("No type @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::ClosingAngle)) {
			log("No '>' @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			log("No '(' @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr expr = takeExpression(tokens);

		if (!expr) {
			log("No subexpression @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			log("No ')' @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr prime = takePrime(tokens);

		if (!prime) {
			log("No prime @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr out = ASTNode::make(NodeType::CastExpression, *cast);
		type->reparent(out);
		expr->reparent(out);
		prime->reparent(out);

		saver.cancel();
		return out;
	}

	ASTNodePtr Parser::takePostfixPrime(std::span<const Token> &tokens) {
		auto log = logger("takePostfixPrime");
		Saver saver{tokens};

		const Token *oper = take(tokens, TokenType::DoublePlus);

		if (!oper) {
			oper = take(tokens, TokenType::DoubleMinus);
			if (!oper) {
				log("No operator @ {}", tokens.front());
				return nullptr;
			}
		}

		ASTNodePtr prime = takePrime(tokens);

		if (!prime) {
			log("No prime @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr out = ASTNode::make(NodeType::PostfixPrime, *oper);
		prime->reparent(out);

		saver.cancel();
		return out;
	}

	ASTNodePtr Parser::takeSizeExpression(std::span<const Token> &tokens) {
		auto log = logger("takeSizeExpression");
		Saver saver{tokens};

		const Token *token = take(tokens, TokenType::Size);

		if (!token) {
			log("No '#size' @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			log("No '(' @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr expr = takeExpression(tokens);

		if (!expr) {
			log("No subexpression @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			log("No ')' @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr prime = takePrime(tokens);

		if (!prime) {
			log("No prime @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr out = ASTNode::make(NodeType::SizeExpression, *token);
		expr->reparent(out);
		prime->reparent(out);

		saver.cancel();
		return out;
	}

	ASTNodePtr Parser::takeNewExpression(std::span<const Token> &tokens) {
		auto log = logger("takeNewExpression");
		Saver saver{tokens};

		const Token *token = take(tokens, TokenType::New);

		if (!token) {
			log("No 'new' @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr type = takeType(tokens, nullptr);

		if (!type) {
			log("No type @ {}", tokens.front());
			return nullptr;
		}

		if (take(tokens, TokenType::OpeningSquare)) {
			log("Found '['");
			ASTNodePtr expr = takeExpression(tokens);

			if (!expr) {
				log("No subexpression @ {}", tokens.front());
				return nullptr;
			}

			if (!take(tokens, TokenType::ClosingSquare)) {
				log("No ']' @ {}", tokens.front());
				return nullptr;
			}

			ASTNodePtr prime = takePrime(tokens);

			if (!prime) {
				log("No prime @ {}", tokens.front());
				return nullptr;
			}

			ASTNodePtr out = ASTNode::make(NodeType::ArrayNewExpression, *token);
			type->reparent(out);
			expr->reparent(out);
			prime->reparent(out);
			saver.cancel();
			return out;
		}

		if (take(tokens, TokenType::OpeningParen)) {
			log("Found '('");
			ASTNodePtr list = takeArgumentList(tokens);

			if (!list) {
				log("No argument list @ {}", tokens.front());
				return nullptr;
			}

			if (!take(tokens, TokenType::ClosingParen)) {
				log("No ')' @ {}", tokens.front());
				return nullptr;
			}

			ASTNodePtr prime = takePrime(tokens);

			if (!prime) {
				log("No prime @ {}", tokens.front());
				return nullptr;
			}

			ASTNodePtr out = ASTNode::make(NodeType::SingleNewExpression, *token);
			type->reparent(out);
			prime->reparent(out);
			list->reparent(out);

			saver.cancel();
			return out;
		}

		log("Short new expression");
		ASTNodePtr prime = takePrime(tokens);

		if (!prime) {
			log("No prime @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr out = ASTNode::make(NodeType::SingleNewExpression, *token);
		type->reparent(out);
		prime->reparent(out);

		saver.cancel();
		return out;
	}
}
