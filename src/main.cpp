#include "mead/Lexer.h"

#include <format>
#include <iostream>
#include <print>

int main(int, char **) {
	using namespace mead;

	Lexer lexer;
	std::print("Success: {}\nTokens:\n", lexer.lex(" foobar 0'621.0e6 0x64'42'00 \"hello \\\"world\\\"\\n?\" "));
	for (const Token &token : lexer.tokens) {
		std::print("\t{}\n", token);
	}
}
