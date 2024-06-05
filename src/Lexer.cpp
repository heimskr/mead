#include "mead/Lexer.h"

#include <algorithm>
#include <cassert>
#include <print>

namespace {
#define PUNCT "!\"#$%&'()*+,\\-./:;<=>?@[\\\\\\]^_`\\s{|}~"
	re2::RE2 floatingPattern{"^(\\d[\\d']*\\.\\d+(e[\\-+]?\\d+))"};
	re2::RE2 integerPattern{"^(([1-9][\\d']+)|(0x[\\da-f][\\d'a-f]*)|(0[0-7']*))"};
	re2::RE2 stringPattern{"^(\"(\\\\[\\\\abenrt\"]|[^\\\\\"])*\")"};
	re2::RE2 identifierPattern{"^([^0-9" PUNCT "][^" PUNCT "]*)"};

	std::string_view trimLeft(std::string_view string) {
		if (size_t pos = string.find_first_not_of(" \f\n\r\t\v"); pos != std::string_view::npos)
			return string.substr(pos);

		return {};
	}
}

namespace mead {
	const LexerRule Lexer::floatingRule{&floatingPattern, TokenType::Floating};
	const LexerRule Lexer::integerRule{&integerPattern, TokenType::Integer};
	const LexerRule Lexer::stringRule{&stringPattern, TokenType::String};
	const LexerRule Lexer::identifierRule{&identifierPattern, TokenType::Identifier};

	Token::Token(TokenType type, std::string value):
		type(type), value(std::move(value)) {}

	LexerRule::LexerRule(const re2::RE2 *regex, TokenType type):
		type(type), regex(regex) {}

	bool LexerRule::attempt(std::string_view input) {
		return succeeded = re2::RE2::PartialMatch(input, *regex, &match);
	}

	LexerRule::operator bool() const {
		return succeeded;
	}

	bool LexerRule::operator<(const LexerRule &other) const {
		if (this == &other)
			return false;

		if (succeeded != other.succeeded)
			return succeeded;

		return match.size() > other.match.size();
	}

	Lexer::Lexer() = default;

	bool Lexer::lex(std::string_view input) {
		for (input = trimLeft(input); !input.empty() && next(input); input = trimLeft(input));
		return trimLeft(input).empty();
	}

	bool Lexer::next(std::string_view &input) {
		std::print("input: \"{}\"\n", input);
		if (input.empty())
			return false;

		std::vector<LexerRule> rules = getRules();
		assert(!rules.empty());

		for (LexerRule &rule : rules) {
			bool success = rule.attempt(input);
			std::print("Rule type: {}, result: {}, match: \"{}\"\n", int(rule.type), success, rule.match);
		}

		std::ranges::sort(rules, std::less<LexerRule>{});

		if (LexerRule &best = rules.front()) {
			assert(!best.match.empty());
			input = input.substr(best.match.size());
			tokens.emplace_back(best.type, std::move(best.match));
			return true;
		}

		return false;
	}

	std::vector<LexerRule> Lexer::getRules() {
		return {floatingRule, integerRule, stringRule, identifierRule};
	}
}
