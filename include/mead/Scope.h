#pragma once

#include "mead/Variable.h"

#include <map>
#include <memory>

namespace mead {
	class Program;
	class Variable;

	class Scope {
		private:
			std::map<std::string, std::shared_ptr<Variable>> variables;
			std::weak_ptr<Program> weakProgram;

		public:
			Scope(std::weak_ptr<Program>);

			std::shared_ptr<Program> getProgram() const;
			std::shared_ptr<Variable> getVariable(const std::string &name) const;
			/** Returns whether the variable was successfully inserted. */
			bool insertVariable(const std::string &name, std::shared_ptr<Variable> variable);

			inline const auto & getVariables() const { return variables; }

			/** Returns whether the variable was successfully inserted. */
			template <typename... Args>
			requires (sizeof...(Args) != 1 || !std::same_as<std::decay_t<std::tuple_element<0, std::tuple<Args...>>>, std::shared_ptr<Variable>>)
			bool insertVariable(const std::string &name, Args &&...args) {
				return insertVariable(name, std::make_shared<Variable>(name, std::forward<Args>(args)...));
			}
	};
}
