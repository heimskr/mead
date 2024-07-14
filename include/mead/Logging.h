#pragma once

#include <format>
#include <iostream>
#include <print>
#include <string>

namespace mead {
	template <typename... Args>
	std::ostream & SUCCESS(std::ostream &stream, const std::format_string<Args...> &format, Args &&...args) {
		std::print(stream, "\x1b[2m[\x1b[22;32m🗸\x1b[39;2m]\x1b[22m ");
		std::println(stream, format, std::forward<Args>(args)...);
		return stream;
	}

	template <typename... Args>
	std::ostream & ERROR(std::ostream &stream, const std::format_string<Args...> &format, Args &&...args) {
		std::print(stream, "\x1b[2m[\x1b[22;31mx\x1b[39;2m]\x1b[22m ");
		std::println(stream, format, std::forward<Args>(args)...);
		return stream;
	}

	template <typename... Args>
	std::ostream & WARN(std::ostream &stream, const std::format_string<Args...> &format, Args &&...args) {
		std::print(stream, "\x1b[2m[\x1b[22;33m~\x1b[39;2m]\x1b[22m ");
		std::println(stream, format, std::forward<Args>(args)...);
		return stream;
	}

	template <typename... Args>
	std::ostream & INFO(std::ostream &stream, const std::format_string<Args...> &format, Args &&...args) {
		std::print(stream, "\x1b[2m[\x1b[22;36mi\x1b[39;2m]\x1b[22m ");
		std::println(stream, format, std::forward<Args>(args)...);
		return stream;
	}

	template <typename... Args>
	std::ostream & SUCCESS(const std::format_string<Args...> &format, Args &&...args) {
		return SUCCESS(std::cerr, format, std::forward<Args>(args)...);
	}

	template <typename... Args>
	std::ostream & ERROR(const std::format_string<Args...> &format, Args &&...args) {
		return ERROR(std::cerr, format, std::forward<Args>(args)...);
	}

	template <typename... Args>
	std::ostream & WARN(const std::format_string<Args...> &format, Args &&...args) {
		return WARN(std::cerr, format, std::forward<Args>(args)...);
	}

	template <typename... Args>
	std::ostream & INFO(const std::format_string<Args...> &format, Args &&...args) {
		return INFO(std::cerr, format, std::forward<Args>(args)...);
	}
}
