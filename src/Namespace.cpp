#include "mead/Function.h"
#include "mead/Logging.h"
#include "mead/Namespace.h"
#include "mead/Type.h"

namespace mead {
	Namespace::Namespace(std::string name, std::weak_ptr<Namespace> parent):
		Symbol(name), weakParent(std::move(parent)), name(std::move(name)) {}

	std::string Namespace::getFullName() const {
		if (auto parent = weakParent.lock())
			return parent->getFullName() + "::" + name;
		return name;
	}

	std::shared_ptr<Namespace> Namespace::getNamespace(const std::string &name, bool create) {
		auto iter = namespaces.find(name);

		if (iter != namespaces.end()) {
			return iter->second;
		}

		if (create) {
			auto new_namespace = std::make_shared<Namespace>(name, weak_from_this());
			namespaces.emplace(name, new_namespace);
			return new_namespace;
		}

		return nullptr;
	}

	std::shared_ptr<Type> Namespace::getType(const std::string &name) const {
		if (auto iter = types.find(name); iter != types.end())
			return iter->second;
		if (auto parent = weakParent.lock())
			return parent->getType(name);
		return nullptr;
	}

	bool Namespace::insertType(const std::string &name, const std::shared_ptr<Type> &type) {
		if (types.emplace(name, type).second) {
			allSymbols[name] = type;
			return true;
		}

		return false;
	}

	bool Namespace::insertFunction(const std::string &name, const std::shared_ptr<Function> &function) {
		if (functions.emplace(name, function).second) {
			allSymbols[name] = function;
			return true;
		}

		return false;
	}
}
