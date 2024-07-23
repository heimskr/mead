#pragma once

#include "mead/Formattable.h"
#include "mead/Scope.h"
#include "mead/Symbol.h"

#include <memory>
#include <string>
#include <vector>

namespace mead {
	class BasicBlock;
	class Program;
	class Type;

	class Function: public Symbol, public Formattable {
		private:
			std::weak_ptr<Program> weakProgram;
			std::shared_ptr<Type> returnType;
			std::vector<std::shared_ptr<Type>> argumentTypes;
			std::shared_ptr<BasicBlock> entryBlock;
			std::shared_ptr<BasicBlock> exitBlock;
			std::vector<std::shared_ptr<BasicBlock>> blocks;
			std::shared_ptr<Scope> scope;

			void initBlocks();

		public:
			Function(const std::shared_ptr<Program> &program, std::string name, std::shared_ptr<Type> return_type, std::vector<std::shared_ptr<Type>> argument_types);

			Function(const Function &) = delete;
			Function(Function &&) = delete;

			Function & operator=(const Function &) = delete;
			Function & operator=(Function &&) = delete;

			std::shared_ptr<BasicBlock> addBlock();

			inline const auto & getReturnType() const { return returnType; }
			inline const auto & getArgumentTypes() const { return argumentTypes; }
			inline auto getEntryBlock() const { return entryBlock; }
			inline auto getExitBlock() const { return exitBlock; }
			inline auto & getScope() { return scope; }
			inline const auto & getScope() const { return scope; }

			std::format_context::iterator formatTo(std::format_context &) const override;
	};

	using FunctionPtr = std::shared_ptr<Function>;
}
