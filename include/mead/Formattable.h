#pragma once

#include <cassert>
#include <format>
#include <memory>

namespace mead {
	struct Formattable {
		virtual std::format_context::iterator formatTo(std::format_context &) const = 0;
	};
}

template <typename T>
requires std::derived_from<T, mead::Formattable>
struct std::formatter<T> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &formattable, std::format_context &ctx) const {
		return formattable.formatTo(ctx);
	}
};

template <typename T>
requires std::derived_from<T, mead::Formattable>
struct std::formatter<std::shared_ptr<T>> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &ptr, std::format_context &ctx) const {
		assert(ptr);
		return std::format_to(ctx.out(), "{}", *ptr);
	}
};

template <typename T>
requires std::derived_from<T, mead::Formattable>
struct std::formatter<std::weak_ptr<T>> {
	formatter() = default;

	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &weak, std::format_context &ctx) const {
		if (auto ptr = weak.lock())
			return std::format_to(ctx.out(), "{}", ptr);
		return std::format_to(ctx.out(), "(expired)");
	}
};
