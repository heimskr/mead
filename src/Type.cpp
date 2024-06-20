#include "mead/Type.h"

namespace mead {
	Type::Type(NamespacedName name):
		name(std::move(name)) {}
}
