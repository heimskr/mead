#include "mead/Lexer.h"
#include "mead/Parser.h"

#include <format>
#include <iostream>
#include <print>

int main(int, char **) {
	using namespace mead;

	const char *example = R"(foobar 0'621.0e6 0x64'42'00 "hello \"world\"\n?"
		0
		u64 x = 42;
	)";

	example = R"(
		u8 foo = 0x42;

		fn main(argc: i32, argv: u8**) -> i32 {
			return i32(foo);
		}
	)";

	example = R"(
		fn main(argc: i32, argv: u8 const * const * const) -> i32 {
			foo: u8;
			{
				foo: u16;
				{
					foo: u32;
				}
			}
			bar: i64;
		}
	)";

	Lexer lexer;
	std::print("Success: {}\nTokens:\n", lexer.lex(example));
	for (const Token &token : lexer.tokens) {
		std::print("\t{}\n", token);
	}

	Parser parser;
	parser.parse(lexer.tokens);

}
