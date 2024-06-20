#pragma once

#include "mead/QualifiedName.h"
#include "mead/Type.h"

#include <map>

namespace mead {
	class TypeDB {
		private:
			std::map<QualifiedName, TypePtr> types;

		public:
			TypePtr operator[](const QualifiedName &);
			TypePtr at(const QualifiedName &) const;
			TypePtr maybe(const QualifiedName &) const;
	};
}
