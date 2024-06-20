#include "mead/TypeDB.h"

namespace mead {
	TypePtr TypeDB::operator[](const QualifiedName &name) {
		if (auto iter = types.find(name); iter != types.end()) {
			return iter->second;
		}

		return types[name] = Type::make(name);
	}

	TypePtr TypeDB::at(const QualifiedName &name) const {
		if (auto iter = types.find(name); iter != types.end()) {
			return iter->second;
		}

		throw std::out_of_range("Type not found");
	}

	TypePtr TypeDB::maybe(const QualifiedName &name) const {
		if (auto iter = types.find(name); iter != types.end()) {
			return iter->second;
		}

		return nullptr;
	}
}
