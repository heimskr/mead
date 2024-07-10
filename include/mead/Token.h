#pragma once

#include <format>
#include <string>

namespace mead {
	enum class TokenType {
		Invalid,
		FloatingLiteral, IntegerLiteral, StringLiteral, CharLiteral,
		IntegerType, Void, Const, Star, Semicolon, Equals, DoubleAmpersand, Ampersand, DoublePipe, Pipe, Arrow, DoubleColon, Colon, Comma,
		DoublePlus, DoubleMinus, Plus, Minus, Bang, Tilde, Cast, Slash, Percent, LeftShift, RightShift, Spaceship, Leq, Geq,
		DoubleEquals, NotEqual, Xor, PlusAssign, MinusAssign, StarAssign, SlashAssign, PercentAssign, LeftShiftAssign, RightShiftAssign,
		AmpersandAssign, XorAssign, PipeAssign, DoubleAmpersandAssign, DoublePipeAssign,
		OpeningSquare, ClosingSquare, OpeningParen, ClosingParen, OpeningBrace, ClosingBrace, OpeningAngle, ClosingAngle,
		Fn, Sizeof, New, Dot, Delete,
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

template <>
struct std::formatter<mead::SourceLocation> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &location, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "[{}:{}]", location.line, location.column);
	}
};

template <>
struct std::formatter<mead::Token> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &token, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "({}: \"{}\" @ {})", int(token.type), token.value, token.location);
	}
};


template <>
struct std::formatter<const mead::Token *> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto *token, std::format_context &ctx) const {
		if (token) {
			return std::format_to(ctx.out(), "{}", *token);
		}

		return std::format_to(ctx.out(), "(null token)");
	}
};
