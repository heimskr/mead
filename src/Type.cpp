#include "mead/Type.h"

namespace mead {
	Type::Type(QualifiedName name):
		name(std::move(name)) {}
}
