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
	class QualifiedType;

	class Parser {
		private:
			std::vector<ASTNodePtr> astNodes;
			TypeDB typeDB;

		public:
			Parser();

			void parse(std::span<const Token> tokens);

		private:
			ASTNodePtr add(ASTNodePtr);
			const Token * peek(std::span<const Token> tokens, TokenType token_type);
			const Token * take(std::span<const Token> &tokens, TokenType token_type);
			ASTNodePtr takeFunctionPrototype(std::span<const Token> &tokens);
			ASTNodePtr takeFunctionDeclaration(std::span<const Token> &tokens);
			ASTNodePtr takeFunctionDefinition(std::span<const Token> &tokens);
			ASTNodePtr takeIdentifier(std::span<const Token> &tokens);
			ASTNodePtr takeTypedVariable(std::span<const Token> &tokens);
			ASTNodePtr takeBlock(std::span<const Token> &tokens);
			ASTNodePtr takeStatement(std::span<const Token> &tokens);
			ASTNodePtr takeType(std::span<const Token> &tokens, QualifiedType *);
			ASTNodePtr takeStar(std::span<const Token> &tokens);
			ASTNodePtr takeAmpersand(std::span<const Token> &tokens);
			std::optional<std::string> takeIdentifierPure(std::span<const Token> &tokens);
			ASTNodePtr takeVariableDeclaration(std::span<const Token> &tokens);
	};
}
