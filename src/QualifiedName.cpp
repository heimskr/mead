#include "mead/QualifiedName.h"

namespace mead {
	QualifiedName::QualifiedName() = default;

	QualifiedName::QualifiedName(std::span<const std::string> namespaces, std::string name):
		namespaces(namespaces.begin(), namespaces.end()), name(std::move(name)) {}

	QualifiedName::operator std::string() const {
		std::string out;
		for (const std::string &namespace_ : namespaces) {
			out += namespace_;
			out += "::";
		}
		out += name;
		return out;
	}

	bool QualifiedName::operator<(const QualifiedName &other) const {
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