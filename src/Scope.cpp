#include "mead/Scope.h"

#include <cassert>

namespace mead {
	Scope::Scope(std::weak_ptr<Program> program):
		weakProgram(std::move(program)) {}

	std::shared_ptr<Program> Scope::getProgram() const {
		auto program = weakProgram.lock();
		assert(program);
		return program;
	}

	std::shared_ptr<Variable> Scope::getVariable(const std::string &name) const {
		if (auto iter = variables.find(name); iter != variables.end())
			return iter->second;
		return {};
	}

	bool Scope::insertVariable(const std::string &name, std::shared_ptr<Variable> variable) {
		return variables.emplace(name, std::move(variable)).second;
	}
}
