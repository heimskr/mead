#pragma once

#include "mead/NamespacedName.h"

#include <memory>

namespace mead {
	class Type {
		public:
			NamespacedName name;

			Type(NamespacedName name);

			template <typename... Args>
			static std::shared_ptr<Type> make(Args &&...args) {
				return std::make_shared<Type>(std::forward<Args>(args)...);
			}
	};

	using TypePtr = std::shared_ptr<Type>;
}