#include "mead/Lexer.h"

#include <algorithm>
#include <cassert>
#include <print>

namespace {
	re2::RE2 floatingLiteralPattern{"^(\\d[\\d']*\\.\\d+([eE][\\-+]?\\d+))"};
	re2::RE2 integerLiteralPattern{"^(([1-9][\\d']*)|(0x[\\da-fA-F][\\d'a-fA-F]*)|(0[0-7']*))"};
	re2::RE2 stringLiteralPattern{"^(\"(\\\\[\\\\0abefnrt\"]|[^\\\\\"])*\")"};
	re2::RE2 charLiteralPattern{"^('(\\\\\\\\|\\\\[0abefnrt']|[^\\\\']|\\\\x[0-9a-fA-F]+)')"};
	re2::RE2 integerTypePattern{"^([iu](8|16|32|64))"};
	re2::RE2 castPattern{"^((static|dynamic|reinterpret|const)_cast)"};
#define PUNCT "!\"#%&'()*+,\\-./:;<=>?@[\\\\\\]\\^_`\\s{|}~" // Intentionally excludes $
	re2::RE2 identifierPattern{"^([^0-9" PUNCT "][^" PUNCT "]*)"};
}

namespace mead {
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
		floatingLiteralRule(TokenType::FloatingLiteral, &floatingLiteralPattern),
		integerLiteralRule(TokenType::IntegerLiteral, &integerLiteralPattern),
		stringLiteralRule(TokenType::StringLiteral, &stringLiteralPattern),
		charLiteralRule(TokenType::CharLiteral, &charLiteralPattern),
		integerTypeRule(TokenType::IntegerType, &integerTypePattern),
		identifierRule(TokenType::Identifier, &identifierPattern),
		castRule(TokenType::Cast, &castPattern) {}

	bool Lexer::lex(std::string_view input) {
		for (input = advanceWhitespace(input); !input.empty() && next(input); input = advanceWhitespace(input));
		return advanceWhitespace(input).empty();
	}

	bool Lexer::next(std::string_view &input) {
		// std::println("input: \"{}\"", input);
		if (input.empty())
			return false;

		std::vector<LexerRule *> rules = getRules();
		assert(!rules.empty());

		for (LexerRule *rule : rules) {
			rule->attempt(input);
			// bool success = rule->attempt(input);
			// std::println("Rule type: {}, result: {}, match: \"{}\"", int(rule->type), success, rule->match);
		}

		std::ranges::sort(rules, [](const auto &left, const auto &right) {
			return *left < *right;
		});

		bool out = false;

		if (LexerRule *best = rules.front()) {
			if (best->match.empty()) {
				std::println("Input[{}]", input);
				if (auto *literal = dynamic_cast<LiteralLexerRule *>(best)) {
					std::println("Literal[{}]", literal->literal);
				} else if (auto *regex = dynamic_cast<RegexLexerRule *>(best)) {
					std::println("Regex[{}]", regex->regex->pattern());
				}
				assert(!best->match.empty());
			}
			input = input.substr(best->match.size());
			SourceLocation location = currentLocation;
			advance(best->match);
			tokens.emplace_back(best->type, std::move(best->match), location);
			out = true;
		}

		for (LexerRule *rule : rules) {
			rule->reset();
		}

		return out;
	}

	std::vector<LexerRule *> Lexer::getRules() {
		return {
			&floatingLiteralRule,
			&integerLiteralRule,
			&stringLiteralRule,
			&charLiteralRule,
			&integerTypeRule,
			&voidRule,
			&starRule,
			&semicolonRule,
			&equalsRule,
			&doubleAmpersandRule,
			&ampersandRule,
			&doublePipeRule,
			&pipeRule,
			&openingSquareRule,
			&closingSquareRule,
			&openingParenRule,
			&closingParenRule,
			&openingBraceRule,
			&closingBraceRule,
			&openingAngleRule,
			&closingAngleRule,
			&fnRule,
			&arrowRule,
			&doubleColonRule,
			&colonRule,
			&commaRule,
			&constRule,
			&doublePlusRule,
			&doubleMinusRule,
			&plusRule,
			&minusRule,
			&bangRule,
			&tildeRule,
			&slashRule,
			&percentRule,
			&leftShiftRule,
			&rightShiftRule,
			&leqRule,
			&geqRule,
			&doubleEqualsRule,
			&notEqualRule,
			&xorRule,
			&plusAssignRule,
			&minusAssignRule,
			&starAssignRule,
			&slashAssignRule,
			&percentAssignRule,
			&leftShiftAssignRule,
			&rightShiftAssignRule,
			&ampersandAssignRule,
			&xorAssignRule,
			&pipeAssignRule,
			&doubleAmpersandAssignRule,
			&doublePipeAssignRule,
			&castRule,
			&dotRule,
			&sizeofRule,
			&newRule,
			&deleteRule,
			&ifRule,
			&elseRule,
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

			if (ch == '\n') {
				advanceLine();
			} else {
				advanceColumn();
			}

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
