#pragma once

#include <re2/re2.h>

#include <string>
#include <vector>

namespace mead {
	enum class TokenType {
		Floating, Integer, String, Identifier,
	};

	struct Token {
		TokenType type;
		std::string value;

		Token(TokenType type, std::string value);
	};

	struct LexerRule {
		TokenType type;
		bool succeeded = false;
		const re2::RE2 *regex;
		std::string match;

		explicit LexerRule(const re2::RE2 *regex, TokenType type);

		bool attempt(std::string_view input);
		explicit operator bool() const;
		bool operator<(const LexerRule &) const;
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
			static const LexerRule floatingRule;
			static const LexerRule integerRule;
			static const LexerRule stringRule;
			static const LexerRule identifierRule;

			std::vector<LexerRule> getRules();
	};
}

template <>
struct std::formatter<mead::Token> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &token, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "({}, \"{}\")", int(token.type), token.value);
	}
};
