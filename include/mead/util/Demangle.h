#pragma once

#include <string>

namespace mead {
	std::string demangle(const char *);
}

#define DEMANGLE(x) ::mead::demangle(typeid(x).name())
