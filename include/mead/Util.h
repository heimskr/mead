#pragma once

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
