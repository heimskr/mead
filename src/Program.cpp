#include "mead/Namespace.h"
#include "mead/Program.h"

namespace mead {
	Program::Program():
		globalNamespace(std::make_shared<Namespace>("")) {}
}
