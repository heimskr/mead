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

			~Saver() {
				if (automatic) {
					restore();
				}
			}

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
