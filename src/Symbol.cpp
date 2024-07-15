#include "mead/Symbol.h"

namespace mead {
	Symbol::Symbol(std::string name):
		name(std::move(name)) {}
}
