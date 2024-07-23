#include "mead/Logging.h"
#include "mead/Scope.h"

#include <cassert>

namespace mead {
	Scope::Scope(std::weak_ptr<Program> program):
		weakProgram(std::move(program)), depth(-1) {}

	Scope::Scope(const std::shared_ptr<Scope> &parent):
		weakProgram(parent->weakProgram), weakParent(parent), depth(parent->depth + 1) {}

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
		INFO("Inserting variable {} with name {} into scope of depth {}", variable, name, depth);
		return variables.emplace(name, std::move(variable)).second;
	}

	std::shared_ptr<Scope> Scope::addScope() {
		auto out = std::make_shared<Scope>(shared_from_this());
		subscopes.emplace_back(out);
		return out;
	}
}
