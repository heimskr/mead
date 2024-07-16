#pragma once

#include "mead/Symbol.h"

#include <map>
#include <memory>
#include <string>

namespace mead {
	class Function;
	class Type;

	class Namespace: public Symbol, public std::enable_shared_from_this<Namespace> {
		private:
			std::weak_ptr<Namespace> weakParent;
			std::string name;
			std::map<std::string, std::shared_ptr<Symbol>> allSymbols;
			std::map<std::string, std::shared_ptr<Namespace>> namespaces;
			std::map<std::string, std::shared_ptr<Type>> types;
			std::map<std::string, std::shared_ptr<Function>> functions;

		public:
			Namespace(std::string name, std::weak_ptr<Namespace> parent = {});

			std::string getFullName() const;
			std::shared_ptr<Namespace> getNamespace(const std::string &name, bool create = false);
			std::shared_ptr<Type> getType(const std::string &name) const;
			/** Returns whether the type was successfully inserted. */
			bool insertType(const std::string &name, const std::shared_ptr<Type> &);
			bool insertFunction(const std::string &name, const std::shared_ptr<Function> &);
	};

	using NamespacePtr = std::shared_ptr<Namespace>;
}
