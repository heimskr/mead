#include "mead/BasicBlock.h"
#include "mead/Function.h"
#include "mead/Logging.h"
#include "mead/Program.h"
#include "mead/Scope.h"
#include "mead/Type.h"
#include "mead/Util.h"

namespace mead {
	Function::Function(const std::shared_ptr<Program> &program, std::string name, std::shared_ptr<Type> return_type, std::vector<std::shared_ptr<Type>> argument_types):
	Symbol(std::move(name)), weakProgram(program), returnType(std::move(return_type)), argumentTypes(std::move(argument_types)), scope(std::make_shared<Scope>(program->getGlobalScope())) {
		initBlocks();
	}

	void Function::initBlocks() {
		entryBlock = std::make_shared<BasicBlock>(*this);
		exitBlock = std::make_shared<BasicBlock>(*this);
	}

	std::shared_ptr<BasicBlock> Function::addBlock() {
		return blocks.emplace_back(std::make_shared<BasicBlock>(*this));
	}

	std::format_context::iterator Function::formatTo(std::format_context &ctx) const {
		return std::format_to(ctx.out(), "fn {}({}) -> {}", name, join(argumentTypes), returnType);
	}
}
