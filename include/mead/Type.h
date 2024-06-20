#pragma once

#include "mead/QualifiedName.h"

#include <memory>

namespace mead {
	class Type {
		public:
			QualifiedName name;

			Type(QualifiedName name);

			template <typename... Args>
			static std::shared_ptr<Type> make(Args &&...args) {
				return std::make_shared<Type>(std::forward<Args>(args)...);
			}
	};

	using TypePtr = std::shared_ptr<Type>;
}