#pragma once

#include <re2/re2.h>

#include <string>
#include <vector>

namespace mead {
	enum class TokenType {
		FloatingLiteral, IntegerLiteral, StringLiteral,
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

			RegexLexerRule floatingRule;
			RegexLexerRule integerRule;
			RegexLexerRule stringRule;
			RegexLexerRule identifierRule;
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
