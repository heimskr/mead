#include "mead/Parser.h"
#include "mead/QualifiedType.h"
#include "mead/Util.h"

#include <cassert>
#include <print>

namespace mead {
	Parser::Parser() = default;

	void Parser::parse(std::span<const Token> tokens) {
		for (;;) {
			if (tokens.empty()) {
				return;
			}

			if (ASTNodePtr node = takeVariableDeclaration(tokens)) {
				add(node);
			// } else if (ASTNodePtr node = takeVariableDefinition(tokens)) {
			// 	add(node);
			} else if (ASTNodePtr node = takeFunctionDeclaration(tokens)) {
				add(node);
			} else if (ASTNodePtr node = takeFunctionDefinition(tokens)) {
				add(node);
			} else {
				std::println("Failed token: {}", tokens.front());
				throw std::runtime_error("Parsing failed");
			}

			std::println("Added token.");
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

		if (!take(tokens, TokenType::FnKeyword)) {
			std::println("FunctionPrototype: \e[31mno FnKeyword\e[39m");
			return nullptr;
		}

		ASTNodePtr name = takeIdentifier(tokens);

		if (!name) {
			std::println("FunctionPrototype: \e[31mno Identifier\e[39m");
			return nullptr;
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			std::println("FunctionPrototype: \e[31mno OpeningParen\e[39m");
			return nullptr;
		}

		name->reparent(node);

		if (peek(tokens, TokenType::ClosingParen)) {
			saver.cancel();
			return node;
		}

		do {
			ASTNodePtr variable = takeTypedVariable(tokens);

			if (!variable) {
				std::println("FunctionPrototype: \e[31mbad variable\e[39m @ {}", tokens.front());
				return nullptr;
			}


			variable->reparent(node);
		} while (take(tokens, TokenType::Comma));

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
			std::println("FunctionDefinition: \e[31mno FunctionPrototype\e[39m");
			return nullptr;
		}

		std::println("\e[32mGot prototype\e[39m. Front token: {}", tokens.front());

		ASTNodePtr block = takeBlock(tokens);

		if (!block) {
			std::println("FunctionDefinition: \e[31mno Block\e[39m");
			return nullptr;
		}

		std::println("\e[32mGot block\e[39m");

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
			std::println("TypedVariable: \e[31mno Identifier\e[39m @ {}", tokens.front());
			return nullptr;
		}

		if (!take(tokens, TokenType::Colon)) {
			std::println("TypedVariable: \e[31mno Colon\e[39m @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr type = takeType(tokens, nullptr);

		if (!type) {
			std::println("TypedVariable: \e[31mno Type\e[39m @ {}", tokens.front());
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
			std::println("Block: \e[31mno OpeningBrace\e[39m @ {}", tokens.front());
			return nullptr;
		}

		ASTNodePtr node = ASTNode::make(NodeType::Block, saver->front());

		while (!take(tokens, TokenType::ClosingBrace)) {
			ASTNodePtr statement = takeStatement(tokens);

			if (!statement) {
				std::println("Block: \e[31mno Statement\e[39m @ {}", tokens.front());
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
			std::println("Front: {}", tokens.front());
			if (const Token *token = take(tokens, TokenType::Const)) {
				std::println("Found const");
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
				std::println("Found star");
				node->add(NodeType::Pointer, *star);
				pointer_consts.push_back(false);
			} else if (const Token *ampersand = take(tokens, TokenType::Ampersand)) {
				std::println("Found ampersand");
				if (ref_found) {
					return nullptr;
				}

				node->add(NodeType::Reference, *ampersand);
				is_const = false;
				is_reference = true;
				ref_found = true;
			} else {
				std::println("Breaking @ {}", tokens.front());
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

}
