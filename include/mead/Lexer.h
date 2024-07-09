#pragma once

#include "mead/Token.h"

#include <re2/re2.h>

#include <memory>
#include <string>
#include <vector>

namespace mead {
	struct LexerRule {
		TokenType type;
		bool succeeded = false;
		std::string match;

		LexerRule(TokenType type);
		virtual ~LexerRule();

		explicit operator bool() const;
		bool operator<(const LexerRule &) const;
		virtual bool attempt(std::string_view input) = 0;
		void reset();
	};

	struct RegexLexerRule : LexerRule {
		const re2::RE2 *regex;

		RegexLexerRule(TokenType type, const re2::RE2 *regex);
		bool attempt(std::string_view input) final;
	};

	struct LiteralLexerRule : LexerRule {
		std::string literal;

		LiteralLexerRule(TokenType type, std::string_view literal);
		bool attempt(std::string_view input) final;
	};

	class Lexer {
		public:
			std::vector<Token> tokens;

			Lexer();

			/** Lexes everything in the given string. Returns whether everything was lexable without anything left over. */
			bool lex(std::string_view);

			/** Tries to lex one token and remove it from the string view. */
			bool next(std::string_view &);

		private:
			SourceLocation currentLocation{1, 1};

			std::vector<LexerRule *> getRules();
			void advance(std::string_view);
			std::string_view advanceWhitespace(std::string_view);
			void advanceLine();
			void advanceColumn();

			RegexLexerRule floatingLiteralRule;
			RegexLexerRule integerLiteralRule;
			RegexLexerRule stringLiteralRule;
			RegexLexerRule charLiteralRule;
			RegexLexerRule integerTypeRule;
			RegexLexerRule identifierRule;
			RegexLexerRule castRule;
			LiteralLexerRule voidRule{TokenType::Void, "void"};
			LiteralLexerRule constRule{TokenType::Const, "const"};
			LiteralLexerRule starRule{TokenType::Star, "*"};
			LiteralLexerRule semicolonRule{TokenType::Semicolon, ";"};
			LiteralLexerRule doubleAmpersandRule{TokenType::DoubleAmpersand, "&&"};
			LiteralLexerRule ampersandRule{TokenType::Ampersand, "&"};
			LiteralLexerRule doublePipeRule{TokenType::DoublePipe, "||"};
			LiteralLexerRule pipeRule{TokenType::Pipe, "|"};
			LiteralLexerRule openingSquareRule{TokenType::OpeningSquare, "["};
			LiteralLexerRule closingSquareRule{TokenType::ClosingSquare, "]"};
			LiteralLexerRule openingParenRule{TokenType::OpeningParen, "("};
			LiteralLexerRule closingParenRule{TokenType::ClosingParen, ")"};
			LiteralLexerRule openingBraceRule{TokenType::OpeningBrace, "{"};
			LiteralLexerRule closingBraceRule{TokenType::ClosingBrace, "}"};
			LiteralLexerRule openingAngleRule{TokenType::OpeningAngle, "<"};
			LiteralLexerRule closingAngleRule{TokenType::ClosingAngle, ">"};
			LiteralLexerRule equalsRule{TokenType::Equals, "="};
			LiteralLexerRule fnRule{TokenType::Fn, "fn"};
			LiteralLexerRule arrowRule{TokenType::Arrow, "->"};
			LiteralLexerRule doubleColonRule{TokenType::DoubleColon, "::"};
			LiteralLexerRule colonRule{TokenType::Colon, ":"};
			LiteralLexerRule commaRule{TokenType::Comma, ","};
			LiteralLexerRule doublePlusRule{TokenType::DoublePlus, "++"};
			LiteralLexerRule doubleMinusRule{TokenType::DoubleMinus, "--"};
			LiteralLexerRule plusRule{TokenType::Plus, "+"};
			LiteralLexerRule minusRule{TokenType::Minus, "-"};
			LiteralLexerRule bangRule{TokenType::Bang, "!"};
			LiteralLexerRule tildeRule{TokenType::Tilde, "~"};
			LiteralLexerRule slashRule{TokenType::Slash, "/"};
			LiteralLexerRule percentRule{TokenType::Percent, "%"};
			LiteralLexerRule leftShiftRule{TokenType::LeftShift, "<<"};
			LiteralLexerRule rightShiftRule{TokenType::RightShift, ">>"};
			LiteralLexerRule spaceshipRule{TokenType::Spaceship, "<=>"};
			LiteralLexerRule leqRule{TokenType::Leq, "<="};
			LiteralLexerRule geqRule{TokenType::Geq, ">="};
			LiteralLexerRule doubleEqualsRule{TokenType::DoubleEquals, "=="};
			LiteralLexerRule notEqualRule{TokenType::NotEqual, "!="};
			LiteralLexerRule xorRule{TokenType::Xor, "^"};
			LiteralLexerRule plusAssignRule{TokenType::PlusAssign, "+="};
			LiteralLexerRule minusAssignRule{TokenType::MinusAssign, "-="};
			LiteralLexerRule starAssignRule{TokenType::StarAssign, "*="};
			LiteralLexerRule slashAssignRule{TokenType::SlashAssign, "/="};
			LiteralLexerRule percentAssignRule{TokenType::PercentAssign, "%="};
			LiteralLexerRule leftShiftAssignRule{TokenType::LeftShiftAssign, "<<="};
			LiteralLexerRule rightShiftAssignRule{TokenType::RightShiftAssign, ">>="};
			LiteralLexerRule ampersandAssignRule{TokenType::AmpersandAssign, "&="};
			LiteralLexerRule xorAssignRule{TokenType::XorAssign, "^="};
			LiteralLexerRule pipeAssignRule{TokenType::PipeAssign, "|="};
			LiteralLexerRule doubleAmpersandAssignRule{TokenType::DoubleAmpersandAssign, "&&="};
			LiteralLexerRule doublePipeAssignRule{TokenType::DoublePipeAssign, "||="};
			LiteralLexerRule sizeRule{TokenType::Size, "#size"};
			LiteralLexerRule newRule{TokenType::New, "new"};
	};

	using LexerPtr = std::shared_ptr<Lexer>;
}
