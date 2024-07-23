#include "mead/node/Statement.h"
#include "mead/util/Demangle.h"
#include "mead/Logging.h"

namespace mead {
	bool Statement::compile(Compiler &, Function &, Scope &, std::shared_ptr<BasicBlock>) {
		WARN("Can't compile {}: unimplemented", DEMANGLE(*this));
		return false;
	}
}