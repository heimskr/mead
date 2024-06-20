#include "mead/TypeDB.h"

namespace mead {
	bool TypeDB::insert(TypePtr type) {
		if (auto iter = types.find(type->name); iter != types.end()) {
			return false;
		}

		types[type->name] = type;
		return true;
	}

	TypePtr TypeDB::operator[](const NamespacedName &name) {
		if (auto iter = types.find(name); iter != types.end()) {
			return iter->second;
		}

		return types[name] = Type::make(name);
	}

	TypePtr TypeDB::at(const NamespacedName &name) const {
		if (auto iter = types.find(name); iter != types.end()) {
			return iter->second;
		}

		throw std::out_of_range("Type not found");
	}

	TypePtr TypeDB::maybe(const NamespacedName &name) const {
		if (auto iter = types.find(name); iter != types.end()) {
			return iter->second;
		}

		return nullptr;
	}
}
