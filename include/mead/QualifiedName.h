#pragma once

#include <span>
#include <string>
#include <vector>

namespace mead {
	class QualifiedName {
		public:
			std::vector<std::string> namespaces;
			std::string name;

			QualifiedName();
			QualifiedName(std::span<const std::string> namespaces, std::string name);

			explicit operator std::string() const;

			bool operator<(const QualifiedName &) const;
	};
}