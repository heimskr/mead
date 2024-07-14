#include "mead/Compiler.h"
#include "mead/Lexer.h"
#include "mead/Logging.h"
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
			foo: i32 const*& const = 40 + 2;
			if 0 {
				if 1 {
					return -42;
					foo;
				}
			} else {
				void(1, if 2 { 3, 4, 5; } else { 6, 7, 8; }, 9);
			}
		}
	)";

	example = R"(
		fn dot() -> void {
			foo: i32*;
			bar: i32 = 64;
			foo = bar.&;
			bar = foo.*;
			foo.*.&.*;
			bar.&.*.&;
			1.0.&;
		}
	)";

	example = R"(
		fn compute() -> i32 {
			x: i32 = 1;
			x * 2 + 40;
		}

		global: i32 = compute();
		x: i32 = global;
		y: i32* = x.&;
		z: i32;

		fn main() -> i32 {
			return y.*;
		}
	)";

	Lexer lexer;

	if (!lexer.lex(example)) {
		ERROR("Lexing failed.");
		return 1;
	}

	// std::print("Success: {}\nTokens:\n", lexer.lex(example));
	// for (const Token &token : lexer.tokens) {
	// 	std::print("\t{}\n", token);
	// }

	Parser parser;
	if (std::optional<Token> failure = parser.parse(lexer.tokens)) {
		ERROR("Parsing failed at {}", *failure);
		parser.print();
		return 2;
	} else {
		SUCCESS("Parsed successfully.");
		// for (const auto &node : parser.getNodes()) {
		// 	node->debug();
		// }
	}

	Compiler compiler;

	if (CompilerResult result = compiler.compile(parser.getNodes())) {
		SUCCESS("Success.");
		std::println("{}", result.value());
	} else{
		ERROR("Compilation failed: {}", result.error().first);
		return 3;
	}
}
