#pragma once

#include "mead/Symbol.h"

#include <map>
#include <memory>
#include <string>

namespace mead {
	class Namespace: public Symbol, public std::enable_shared_from_this<Namespace> {
		private:
			std::weak_ptr<Namespace> weakParent;
			std::string name;
			std::map<std::string, std::shared_ptr<Symbol>> allSymbols;
			std::map<std::string, std::shared_ptr<Namespace>> namespaces;

		public:
			Namespace(std::string name, std::weak_ptr<Namespace> parent = {});

			std::string getFullName() const;
			std::shared_ptr<Namespace> getNamespace(const std::string &name, bool create = false);
	};
}
