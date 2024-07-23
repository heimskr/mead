#pragma once

#include "mead/Variable.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace mead {
	class Program;
	class Variable;

	class Scope: public std::enable_shared_from_this<Scope> {
		private:
			std::map<std::string, std::shared_ptr<Variable>> variables;
			std::weak_ptr<Program> weakProgram;
			/** Will be empty for the root scope. */
			std::weak_ptr<Scope> weakParent;
			std::vector<std::shared_ptr<Scope>> subscopes;
			ssize_t depth = 0;

		public:
			Scope(std::weak_ptr<Program>);
			Scope(const std::shared_ptr<Scope> &);

			std::shared_ptr<Program> getProgram() const;
			std::shared_ptr<Variable> getVariable(const std::string &name) const;
			/** Returns whether the variable was successfully inserted. */
			bool insertVariable(const std::string &name, std::shared_ptr<Variable> variable);
			std::shared_ptr<Scope> addScope();

			inline const auto & getVariables() const { return variables; }
			inline const auto & getSubscopes() const { return subscopes; }
			inline auto getDepth() const { return depth; }

			/** Returns whether the variable was successfully inserted. */
			template <typename... Args>
			requires (sizeof...(Args) != 1 || !std::same_as<std::decay_t<std::tuple_element<0, std::tuple<Args...>>>, std::shared_ptr<Variable>>)
			bool insertVariable(const std::string &name, Args &&...args) {
				return insertVariable(name, std::make_shared<Variable>(name, std::forward<Args>(args)...));
			}
	};

	using ScopePtr = std::shared_ptr<Scope>;
}
