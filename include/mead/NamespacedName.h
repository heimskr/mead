#pragma once

#include <span>
#include <string>
#include <vector>

namespace mead {
	class NamespacedName {
		public:
			std::vector<std::string> namespaces;
			std::string name;

			NamespacedName();
			NamespacedName(std::vector<std::string> namespaces, std::string name);
			NamespacedName(std::span<const std::string> namespaces, std::string name);

			explicit operator std::string() const;

			bool operator<(const NamespacedName &) const;
	};
}