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
		baz: i8;

		fn main(argc: i32, argv: u8 const * const * const) -> i32 {
			foo: u8;
			{
				foo: u16;
				{
					foo: u32;
				}
			}
			bar: i64;
			new i8;
		};
	)";

	example = R"(
		fn main() -> i32 {
			"hello"[42] + 2 * 3 - 4 / 5;
		}
	)";

	example = R"(
		fn unadorned() -> i32 {
			1 + 2 + 3 + 4 + 5;
		}
	)";

	example = R"(
		fn complex() -> i32 {
			1 + ++x + y++;
		}
	)";

	example = R"(
		fn pluses() -> i32 {
			1+ +++1++;
			1++-+-+--+-+-++1;
		}
	)";

	example = R"(
		fn def() -> i32 {
			foo: i32 = 40 + 2;
			if 0 {
				if 1 {
					foo;
				}
			} else {
				void(1, if 2 { 3, 4, 5; } else { 6, 7, 8; }, 9);
			}
		}
	)";

	Lexer lexer;

	if (!lexer.lex(example)) {
		std::println("Lexing failed.");
		return 1;
	}

	// std::print("Success: {}\nTokens:\n", lexer.lex(example));
	// for (const Token &token : lexer.tokens) {
	// 	std::print("\t{}\n", token);
	// }

	Parser parser;
	if (std::optional<Token> failure = parser.parse(lexer.tokens)) {
		std::println("Parsing failed at {}", *failure);
		parser.print();
	} else {
		std::println("Parsed successfully.");
		for (const auto &node : parser.getNodes()) {
			node->debug();
		}
	}
}
