#pragma once

#include <charconv>
#include <format>
#include <string>
#include <sstream>

namespace mead {
	template <typename C>
	void join(const C &container, std::format_context &ctx, std::string_view delimiter = ", ") {
		bool first = true;
		for (const auto &item : container) {
			if (first) {
				first = false;
				std::format_to(ctx.out(), "{}", item);
			} else {
				std::format_to(ctx.out(), "{}{}", delimiter, item);
			}
		}
	}

	template <typename C>
	std::string join(const C &container, std::string_view delimiter = ", ") {
		std::stringstream out;
		bool first = true;
		for (const auto &item : container) {
			if (first) {
				first = false;
				std::print(out, "{}", item);
			} else {
				std::print(out, "{}{}", delimiter, item);
			}
		}
		return out.str();
	}

	template <std::integral I>
#ifdef MEAD_PARSENUMBER_DECLARED
	I parseNumber(std::string_view view, int base) {
#else
	I parseNumber(std::string_view view, int base = 10) {
#endif
#define MEAD_PARSENUMBER_DECLARED
		I out{};
		auto result = std::from_chars(view.begin(), view.end(), out, base);
		if (result.ec == std::errc::invalid_argument)
			throw std::invalid_argument("Not an integer: \"" + std::string(view) + "\"");
		return out;
	}

	template <std::floating_point F>
	F parseNumber(std::string_view view) {
#if defined(__APPLE__) && defined(__clang__)
		// Current Clang on macOS seems to hate from_chars for floating point types.
		std::string str(view);
		char *endptr = nullptr;
		double out = strtod(str.c_str(), &endptr);
		if (str.c_str() + str.size() != endptr)
			throw std::invalid_argument("Not a floating point: \"" + str + "\"");
		return out;
#else
		F out{};
		auto result = std::from_chars(view.begin(), view.end(), out);
		if (result.ec == std::errc::invalid_argument)
			throw std::invalid_argument("Not a floating point: \"" + std::string(view) + "\"");
		return out;
#endif
	}

	template <typename T>
	class Saver {
		private:
			T &reference;
			T saved;
			bool automatic;

		public:
			Saver(T &object, bool automatic = true):
				reference(object), saved(object), automatic(automatic) {}

			Saver(const Saver &) = delete;
			Saver(Saver &&) = delete;

			~Saver() {
				if (automatic) {
					restore();
				}
			}

			Saver & operator=(const Saver &) = delete;
			Saver & operator=(Saver &&) = delete;

			T & save() {
				saved = reference;
				return saved;
			}

			T & restore() {
				reference = saved;
				return reference;
			}

			void cancel() {
				automatic = false;
			}

			T & get() {
				return saved;
			}

			const T & get() const {
				return saved;
			}

			T * operator->() {
				return &saved;
			}

			const T * operator->() const {
				return &saved;
			}
	};
}
