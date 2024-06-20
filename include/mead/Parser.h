#pragma once

#include "mead/ASTNode.h"
#include "mead/Token.h"
#include "mead/TypeDB.h"

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace mead {
	class ASTNode;

	class Parser {
		private:
			std::vector<std::shared_ptr<ASTNode>> astNodes;
			TypeDB typeDB;

		public:
			Parser();

			void parse(std::span<const Token> tokens);

		private:
			bool peek(std::span<const Token> tokens, TokenType token_type);
			bool take(std::span<const Token> &tokens, TokenType token_type);
			ASTNodePtr takeFunctionPrototype(std::span<const Token> &tokens);
			ASTNodePtr takeFunctionDeclaration(std::span<const Token> &tokens);
			ASTNodePtr takeFunctionDefinition(std::span<const Token> &tokens);
			ASTNodePtr takeIdentifier(std::span<const Token> &tokens);
			ASTNodePtr takeVariableDeclaration(std::span<const Token> &tokens);
			ASTNodePtr takeBlock(std::span<const Token> &tokens);
			ASTNodePtr takeStatement(std::span<const Token> &tokens);
			ASTNodePtr takeType(std::span<const Token> &tokens);
			ASTNodePtr takeStar(std::span<const Token> &tokens);
			ASTNodePtr takeAmpersand(std::span<const Token> &tokens);
			std::optional<std::string> takeIdentifierPure(std::span<const Token> &tokens);
	};
}
