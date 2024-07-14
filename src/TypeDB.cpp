#include "mead/TypeDB.h"

namespace mead {
	std::map<NamespacedName, TypePtr> TypeDB::getDefaultTypes() {
		return {
			{"void", std::make_shared<VoidType>()},
			{"i8",   std::make_shared<IntType>(8,   true)},
			{"u8",   std::make_shared<IntType>(8,  false)},
			{"i16",  std::make_shared<IntType>(16,  true)},
			{"u16",  std::make_shared<IntType>(16, false)},
			{"i32",  std::make_shared<IntType>(32,  true)},
			{"u32",  std::make_shared<IntType>(32, false)},
			{"i64",  std::make_shared<IntType>(64,  true)},
			{"u64",  std::make_shared<IntType>(64, false)},
		};
	}

	TypeDB::TypeDB():
		types(getDefaultTypes()) {}

	bool TypeDB::insert(TypePtr /* type */) {
		// if (auto iter = types.find(type->name); iter != types.end()) {
		// 	return false;
		// }

		// types[type->name] = type;
		return true;
	}

	TypePtr TypeDB::operator[](const NamespacedName &/* name */) {
		// if (auto iter = types.find(name); iter != types.end()) {
		// 	return iter->second;
		// }

		// return types[name] = Type::make(name);
		return nullptr;
	}

	TypePtr TypeDB::at(const NamespacedName &/* name */) const {
		// if (auto iter = types.find(name); iter != types.end()) {
		// 	return iter->second;
		// }

		// throw std::out_of_range("Type not found");
		return nullptr;
	}

	TypePtr TypeDB::maybe(const NamespacedName &/* name */) const {
		// if (auto iter = types.find(name); iter != types.end()) {
		// 	return iter->second;
		// }

		return nullptr;
	}

	bool TypeDB::contains(const NamespacedName &name) const {
		return types.contains(name);
	}
}
