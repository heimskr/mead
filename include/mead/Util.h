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
}
