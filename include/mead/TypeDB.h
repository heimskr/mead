#pragma once

#include "mead/NamespacedName.h"
#include "mead/Type.h"

#include <map>

namespace mead {
	class TypeDB {
		private:
			std::map<NamespacedName, TypePtr> types;

		public:
			bool insert(TypePtr);
			TypePtr operator[](const NamespacedName &);
			TypePtr at(const NamespacedName &) const;
			TypePtr maybe(const NamespacedName &) const;
	};
}
