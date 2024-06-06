#include "mead/Lexer.h"

#include <algorithm>
#include <cassert>
#include <print>

namespace {
#define PUNCT "!\"#$%&'()*+,\\-./:;<=>?@[\\\\\\]^_`\\s{|}~"
	re2::RE2 floatingLiteralPattern{"^(\\d[\\d']*\\.\\d+(e[\\-+]?\\d+))"};
	re2::RE2 integerLiteralPattern{"^(([1-9][\\d']+)|(0x[\\da-f][\\d'a-f]*)|(0[0-7']*))"};
	re2::RE2 stringLiteralPattern{"^(\"(\\\\[\\\\abenrt\"]|[^\\\\\"])*\")"};
	re2::RE2 identifierPattern{"^([^0-9" PUNCT "][^" PUNCT "]*)"};
}

namespace mead {
	SourceLocation::SourceLocation(size_t line, size_t column):
		line(line), column(column) {}

	Token::Token(TokenType type, std::string value, SourceLocation location):
		type(type), value(std::move(value)), location(location) {}

	LexerRule::LexerRule(TokenType type):
		type(type) {}

	LexerRule::~LexerRule() = default;

	LexerRule::operator bool() const {
		return succeeded;
	}

	bool LexerRule::operator<(const LexerRule &other) const {
		if (this == &other)
			return false;

		if (succeeded != other.succeeded)
			return succeeded;

		if (match.size() == other.match.size())
			return type < other.type;

		return match.size() > other.match.size();
	}

	void LexerRule::reset() {
		succeeded = false;
		match.clear();
	}

	RegexLexerRule::RegexLexerRule(TokenType type, const re2::RE2 *regex):
		LexerRule(type), regex(regex) {}

	bool RegexLexerRule::attempt(std::string_view input) {
		return succeeded = re2::RE2::PartialMatch(input, *regex, &match);
	}

	LiteralLexerRule::LiteralLexerRule(TokenType type, std::string_view literal):
		LexerRule(type), literal(literal) {}

	bool LiteralLexerRule::attempt(std::string_view input) {
		if (input.starts_with(literal)) {
			match = literal;
			return succeeded = true;
		}

		match.clear();
		return succeeded = false;
	}

	Lexer::Lexer():
		floatingRule(TokenType::FloatingLiteral, &floatingLiteralPattern),
		integerRule(TokenType::IntegerLiteral, &integerLiteralPattern),
		stringRule(TokenType::StringLiteral, &stringLiteralPattern),
		identifierRule(TokenType::Identifier, &identifierPattern) {}

	bool Lexer::lex(std::string_view input) {
		for (input = advanceWhitespace(input); !input.empty() && next(input); input = advanceWhitespace(input));
		return advanceWhitespace(input).empty();
	}

	bool Lexer::next(std::string_view &input) {
		std::print("input: \"{}\"\n", input);
		if (input.empty())
			return false;

		std::vector<LexerRule *> rules = getRules();
		assert(!rules.empty());

		for (LexerRule *rule : rules) {
			bool success = rule->attempt(input);
			std::print("Rule type: {}, result: {}, match: \"{}\"\n", int(rule->type), success, rule->match);
		}

		std::ranges::sort(rules, [](const auto &left, const auto &right) {
			return *left < *right;
		});

		bool out = false;

		if (LexerRule *best = rules.front()) {
			assert(!best->match.empty());
			input = input.substr(best->match.size());
			SourceLocation location = currentLocation;
			advance(best->match);
			tokens.emplace_back(best->type, std::move(best->match), location);
			out = true;
		}

		for (LexerRule *rule : rules)
			rule->reset();

		return out;
	}

	std::vector<LexerRule *> Lexer::getRules() {
		return {
			&floatingRule,
			&integerRule,
			&stringRule,
			&identifierRule,
		};
	}

	void Lexer::advance(std::string_view text) {
		for (const char ch : text) {
			if (ch == '\n') {
				advanceLine();
			} else {
				advanceColumn();
			}
		}
	}

	std::string_view Lexer::advanceWhitespace(std::string_view text) {
		while (!text.empty()) {
			char ch = text[0];

			if (!std::isspace(ch))
				return text;

			if (ch == '\n')
				advanceLine();
			else
				advanceColumn();

			text.remove_prefix(1);
		}

		return text;
	}

	void Lexer::advanceLine() {
		++currentLocation.line;
		currentLocation.column = 1;
	}

	void Lexer::advanceColumn() {
		++currentLocation.column;
	}
}
