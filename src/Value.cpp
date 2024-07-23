#include "mead/util/Demangle.h"
#include "mead/Value.h"

namespace mead {
	std::string Value::toString() const {
		return '[' + DEMANGLE(*this) + ']';
	}
}