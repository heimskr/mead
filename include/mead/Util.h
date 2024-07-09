#pragma once

namespace mead {
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

	template <typename T>
	class StackGuard {
		private:
			std::vector<T> &stack;
			size_t initialSize;
			bool canceled = false;

		public:
			template <typename U>
			StackGuard(std::vector<T> &stack, U &&new_item): stack(stack), initialSize(stack.size()) {
				stack.emplace_back(std::forward<U>(new_item));
			}

			StackGuard(const StackGuard &) = delete;
			StackGuard(StackGuard &&) = delete;

			~StackGuard() {
				if (canceled) {
					return;
				}

				if (stack.size() != initialSize + 1) {
					std::println(std::cerr, "Current stack size ({}) isn't the expected value ({})", stack.size(), initialSize + 1);
					std::terminate();
				}

				stack.pop_back();
			}

			StackGuard & operator=(const StackGuard &) = delete;
			StackGuard & operator=(StackGuard &&) = delete;

			void cancel() {
				canceled = true;
			}
	};
}
