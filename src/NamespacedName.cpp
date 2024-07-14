#include "mead/NamespacedName.h"

namespace mead {
	NamespacedName::NamespacedName() = default;

	NamespacedName::NamespacedName(const char *name):
		name(std::move(name)) {}

	NamespacedName::NamespacedName(std::string name):
		name(std::move(name)) {}

	NamespacedName::NamespacedName(std::vector<std::string> namespaces, std::string name):
		namespaces(std::move(namespaces)), name(std::move(name)) {}

	NamespacedName::NamespacedName(std::span<const std::string> namespaces, std::string name):
		NamespacedName(std::vector(namespaces.begin(), namespaces.end()), std::move(name)) {}

	NamespacedName::operator std::string() const {
		std::string out;
		for (const std::string &namespace_ : namespaces) {
			out += namespace_;
			out += "::";
		}
		out += name;
		return out;
	}

	bool NamespacedName::operator<(const NamespacedName &other) const {
		if (this == &other || other.namespaces.size() > namespaces.size()) {
			return false;
		}

		if (other.namespaces.size() < namespaces.size()) {
			return true;
		}

		for (size_t i = 0; i < namespaces.size(); ++i) {
			if (namespaces[i] < other.namespaces[i]) {
				return true;
			}

			if (namespaces[i] > other.namespaces[i]) {
				return false;
			}
		}

		return name < other.name;
	}
}