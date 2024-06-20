#pragma once

#include <string>

namespace mead {
	enum class TokenType {
		Invalid,
		FloatingLiteral, IntegerLiteral, StringLiteral, CharLiteral,
		IntegerType, Void, Const, Star, Semicolon, Equals, DoubleAmpersand, Ampersand, DoublePipe, Pipe, Arrow, Colon, Comma,
		OpeningSquare, ClosingSquare, OpeningParen, ClosingParen, OpeningBrace, ClosingBrace, OpeningAngle, ClosingAngle,
		FnKeyword,
		Identifier,
	};

	struct SourceLocation {
		size_t line{};
		size_t column{};

		SourceLocation();
		SourceLocation(size_t line, size_t column);
	};

	struct Token {
		TokenType type{};
		std::string value;
		SourceLocation location;

		Token();
		Token(TokenType type, std::string value, SourceLocation location);
	};
}
