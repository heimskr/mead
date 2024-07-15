#pragma once

#include "mead/Namespace.h"

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
}
