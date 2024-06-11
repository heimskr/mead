#include "mead/Token.h"

namespace mead {
	SourceLocation::SourceLocation() = default;

	SourceLocation::SourceLocation(size_t line, size_t column):
		line(line), column(column) {}

	Token::Token() = default;

	Token::Token(TokenType type, std::string value, SourceLocation location):
		type(type), value(std::move(value)), location(location) {}
}
