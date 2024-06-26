#include "mead/Parser.h"
#include "mead/QualifiedType.h"
#include "mead/Util.h"

#include <cassert>
#include <print>

namespace mead {
	Parser::Parser() = default;

	std::optional<Token> Parser::parse(std::span<const Token> tokens) {
		for (;;) {
			if (tokens.empty()) {
				return std::nullopt;
			}

			if (ASTNodePtr node = takeVariableDeclaration(tokens)) {
				add(node);
			// } else if (ASTNodePtr node = takeVariableDefinition(tokens)) {
			// 	add(node);
			} else if (ASTNodePtr node = takeFunctionDeclaration(tokens)) {
				add(node);
			} else if (ASTNodePtr node = takeFunctionDefinition(tokens)) {
				add(node);
			} else if (take(tokens, TokenType::Semicolon)) {
				;
			} else {
				return tokens.front();
			}
		}
	}

	ASTNodePtr Parser::add(ASTNodePtr node) {
		astNodes.push_back(node);
		return node;
	}

	const Token * Parser::peek(std::span<const Token> tokens, TokenType token_type) {
		if (tokens.empty() || tokens.front().type != token_type) {
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
		if (tokens.empty()) {
			return nullptr;
		}

		Saver saver{tokens};

		ASTNodePtr node = ASTNode::make(NodeType::FunctionPrototype, tokens.front());

		if (!take(tokens, TokenType::Fn)) {
			return nullptr;
		}

		ASTNodePtr name = takeIdentifier(tokens);

		if (!name) {
			return nullptr;
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			return nullptr;
		}

		name->reparent(node);

		std::vector<ASTNodePtr> variables;

		if (!take(tokens, TokenType::ClosingParen)) {
			do {
				ASTNodePtr variable = takeTypedVariable(tokens);

				if (!variable) {
					return nullptr;
				}


				variables.push_back(std::move(variable));
			} while (take(tokens, TokenType::Comma));

			if (!take(tokens, TokenType::ClosingParen)) {
				return nullptr;
			}
		}

		if (take(tokens, TokenType::Arrow)) {
			if (ASTNodePtr return_type = takeType(tokens, nullptr)) {
				return_type->reparent(node);
			} else {
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
		Saver saver{tokens};

		ASTNodePtr prototype = takeFunctionPrototype(tokens);

		if (!prototype || !take(tokens, TokenType::Semicolon)) {
			return nullptr;
		}

		ASTNodePtr node = ASTNode::make(NodeType::FunctionDeclaration, prototype->token);
		prototype->reparent(node);

		saver.cancel();
		return prototype;
	}

	ASTNodePtr Parser::takeFunctionDefinition(std::span<const Token> &tokens) {
		Saver saver{tokens};

		ASTNodePtr prototype = takeFunctionPrototype(tokens);

		if (!prototype) {
			return nullptr;
		}


		ASTNodePtr block = takeBlock(tokens);

		if (!block) {
			return nullptr;
		}


		ASTNodePtr node = ASTNode::make(NodeType::FunctionDefinition, prototype->token);
		prototype->reparent(node);
		block->reparent(node);

		saver.cancel();
		return node;
	}

	ASTNodePtr Parser::takeIdentifier(std::span<const Token> &tokens) {
		if (!peek(tokens, TokenType::Identifier)) {
			return nullptr;
		}

		const Token &token = tokens.front();
		tokens = tokens.subspan(1);
		return ASTNode::make(NodeType::Identifier, token);
	}

	ASTNodePtr Parser::takeTypedVariable(std::span<const Token> &tokens) {
		if (tokens.empty()) {
			return nullptr;
		}

		Saver saver{tokens};

		ASTNodePtr name = takeIdentifier(tokens);

		if (!name) {
			return nullptr;
		}

		if (!take(tokens, TokenType::Colon)) {
			return nullptr;
		}

		ASTNodePtr type = takeType(tokens, nullptr);

		if (!type) {
			return nullptr;
		}

		ASTNodePtr node = ASTNode::make(NodeType::VariableDeclaration, saver->front());
		name->reparent(node);
		type->reparent(node);

		saver.cancel();
		return node;
	}

	ASTNodePtr Parser::takeBlock(std::span<const Token> &tokens) {
		Saver saver{tokens};

		if (!take(tokens, TokenType::OpeningBrace)) {
			return nullptr;
		}

		ASTNodePtr node = ASTNode::make(NodeType::Block, saver->front());

		while (!take(tokens, TokenType::ClosingBrace)) {
			ASTNodePtr statement = takeStatement(tokens);

			if (!statement) {
				return nullptr;
			}

			statement->reparent(node);
		}

		saver.cancel();
		return node;
	}

	ASTNodePtr Parser::takeStatement(std::span<const Token> &tokens) {
		if (ASTNodePtr node = takeVariableDeclaration(tokens)) {
			return node;
		}

		if (ASTNodePtr node = takeBlock(tokens)) {
			return node;
		}

		return nullptr;
	}

	ASTNodePtr Parser::takeType(std::span<const Token> &tokens, QualifiedType *type_out) {
		if (tokens.empty()) {
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
		Saver saver{tokens};

		ASTNodePtr node = takeTypedVariable(tokens);

		if (!node || !take(tokens, TokenType::Semicolon)) {
			return nullptr;
		}

		saver.cancel();
		return node;
	}

	ASTNodePtr Parser::takeExpression(std::span<const Token> &tokens) {
		Saver saver{tokens};

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

		return nullptr;
	}

	ASTNodePtr Parser::takeConstructorExpression(std::span<const Token> &tokens) {
		if (tokens.empty()) {
			return nullptr;
		}

		const Token &anchor = tokens.front();
		Saver saver{tokens};

		ASTNodePtr type = takeType(tokens, nullptr);

		if (!type) {
			return nullptr;
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			return nullptr;
		}

		ASTNodePtr list = takeExpressionList(tokens);

		if (!list) {
			return nullptr;
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			return nullptr;
		}

		ASTNodePtr out = ASTNode::make(NodeType::ConstructorExpression, anchor);
		type->reparent(out);
		list->reparent(out);
		saver.cancel();
		return out;
	}

	ASTNodePtr Parser::takePrefixExpression(std::span<const Token> &tokens) {
		Saver saver{tokens};

		const Token *oper = take(tokens, TokenType::DoublePlus);

		if (!oper) {
			oper = take(tokens, TokenType::DoubleMinus);
			if (!oper) {
				return nullptr;
			}
		}

		ASTNodePtr subexpr = takeExpression(tokens);

		if (!subexpr) {
			return nullptr;
		}

		ASTNodePtr prime = takePrime(tokens);

		if (!prime) {
			return nullptr;
		}

		ASTNodePtr out = ASTNode::make(NodeType::PrefixExpression, *oper);
		subexpr->reparent(out);
		prime->reparent(out);
		saver.cancel();
		return out;
	}

	ASTNodePtr Parser::takeUnaryPrefixExpression(std::span<const Token> &tokens) {
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
			return nullptr;
		}

		ASTNodePtr expr = takeExpression(tokens);

		if (!expr) {
			return nullptr;
		}

		ASTNodePtr prime = takePrime(tokens);

		if (!prime) {
			return nullptr;
		}

		ASTNodePtr out = ASTNode::make(NodeType::UnaryExpression, *oper);
		expr->reparent(out);
		prime->reparent(out);
		saver.cancel();
		return out;
	}

	ASTNodePtr Parser::takeCastExpression(std::span<const Token> &tokens) {
		Saver saver{tokens};

		const Token *cast = take(tokens, TokenType::Cast);

		if (!cast) {
			return nullptr;
		}

		if (!take(tokens, TokenType::OpeningAngle)) {
			return nullptr;
		}

		ASTNodePtr type = takeType(tokens, nullptr);

		if (!take(tokens, TokenType::ClosingAngle)) {
			return nullptr;
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			return nullptr;
		}

		ASTNodePtr expr = takeExpression(tokens);

		if (!expr) {
			return nullptr;
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			return nullptr;
		}

		ASTNodePtr prime = takePrime(tokens);

		if (!prime) {
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
		Saver saver{tokens};

		const Token *oper = take(tokens, TokenType::DoublePlus);

		if (!oper) {
			oper = take(tokens, TokenType::DoubleMinus);
			if (!oper) {
				return nullptr;
			}
		}

		ASTNodePtr prime = takePrime(tokens);

		if (!prime) {
			return nullptr;
		}

		ASTNodePtr out = ASTNode::make(NodeType::PostfixPrime, *oper);
		prime->reparent(out);
		saver.cancel();
		return out;
	}

	ASTNodePtr Parser::takeSizeExpression(std::span<const Token> &tokens) {
		Saver saver{tokens};

		const Token *token = take(tokens, TokenType::Size);

		if (!token) {
			return nullptr;
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			return nullptr;
		}

		ASTNodePtr expr = takeExpression(tokens);

		if (!expr) {
			return nullptr;
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			return nullptr;
		}

		ASTNodePtr prime = takePrime(tokens);

		if (!prime) {
			return nullptr;
		}

		ASTNodePtr out = ASTNode::make(NodeType::SizeExpression, *token);
		expr->reparent(out);
		prime->reparent(out);
		saver.cancel();
		return out;
	}
}
