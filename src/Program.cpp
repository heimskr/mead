#include "mead/Namespace.h"
#include "mead/Program.h"
#include "mead/Scope.h"
#include "mead/Type.h"

namespace mead {
	Program::Program():
		globalNamespace(std::make_shared<Namespace>("")) {}

	void Program::init() {
		globalScope = std::make_shared<Scope>(weak_from_this());
		Namespace &global = *globalNamespace;

		for (bool is_signed : {true, false}) {
			for (int bit_width : {8, 16, 32, 64}) {
				std::string name = std::format("{}{}", is_signed? 'i' : 'u', bit_width);
				bool inserted = global.insertType(name, std::make_shared<IntType>(bit_width, is_signed));
				assert(inserted);
			}
		}
	}

	std::shared_ptr<Namespace> Program::getGlobalNamespace() const {
		return globalNamespace;
	}

	std::shared_ptr<Scope> Program::getGlobalScope() const {
		return globalScope;
	}
}
