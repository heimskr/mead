#pragma once

#include <re2/re2.h>

#include <string>
#include <vector>

namespace mead {
	enum class TokenType {
		FloatingLiteral, IntegerLiteral, StringLiteral, CharLiteral,
		IntegerType, Void, Const, Star, Semicolon, Equals, DoubleAmpersand, Ampersand, DoublePipe, Pipe,
		OpeningSquare, ClosingSquare, OpeningParen, ClosingParen, OpeningBrace, ClosingBrace, OpeningAngle, ClosingAngle,
		Identifier,
	};

	struct SourceLocation {
		size_t line{};
		size_t column{};

		SourceLocation(size_t line, size_t column);
	};

	struct Token {
		TokenType type;
		std::string value;
		SourceLocation location;

		Token(TokenType type, std::string value, SourceLocation location);
	};

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
	};
}

template <>
struct std::formatter<mead::SourceLocation> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &location, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "[{}:{}]", location.line, location.column);
	}
};

template <>
struct std::formatter<mead::Token> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &token, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "({}: \"{}\" @ {})", int(token.type), token.value, token.location);
	}
};
