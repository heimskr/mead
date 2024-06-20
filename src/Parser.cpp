#include "mead/Parser.h"
#include "mead/Util.h"

namespace mead {
	Parser::Parser() = default;

	void Parser::parse(std::span<const Token> tokens) {

	}

	bool Parser::peek(std::span<const Token> tokens, TokenType token_type) {
		return !tokens.empty() && tokens.front().type == token_type;
	}

	bool Parser::take(std::span<const Token> &tokens, TokenType token_type) {
		if (peek(tokens, token_type)) {
			tokens = tokens.subspan(1);
			return true;
		}

		return false;
	}

	ASTNodePtr Parser::takeFunctionPrototype(std::span<const Token> &tokens) {
		if (tokens.empty()) {
			return nullptr;
		}

		Saver saver{tokens};

		ASTNodePtr node = ASTNode::make(NodeType::FunctionDeclaration, tokens.front());

		if (!take(tokens, TokenType::FnKeyword)) {
			return nullptr;
		}

		ASTNodePtr name = takeIdentifier(tokens);

		if (!name || !take(tokens, TokenType::OpeningParen)) {
			return nullptr;
		}

		name->reparent(node);

		if (peek(tokens, TokenType::ClosingParen)) {
			saver.cancel();
			return node;
		}

		do {
			ASTNodePtr variable = takeVariableDeclaration(tokens);

			if (!variable) {
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

	ASTNodePtr Parser::takeVariableDeclaration(std::span<const Token> &tokens) {
		Saver saver{tokens};
		return nullptr;
	}

	ASTNodePtr Parser::takeBlock(std::span<const Token> &tokens) {
		return nullptr;
	}

	ASTNodePtr Parser::takeStatement(std::span<const Token> &tokens) {
		return nullptr;
	}

	ASTNodePtr Parser::takeType(std::span<const Token> &tokens) {
		Saver saver{tokens};

		std::vector<std::string> pieces;
		// ... get namespaces and such

		return nullptr;
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
