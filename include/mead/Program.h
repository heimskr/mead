#pragma once

#include <memory>

namespace mead {
	class Namespace;
	class Scope;

	class Program: public std::enable_shared_from_this<Program> {
		private:
			std::shared_ptr<Namespace> globalNamespace;
			std::shared_ptr<Scope> globalScope;

		public:
			Program();

			void init();

			std::shared_ptr<Namespace> getGlobalNamespace() const;
			std::shared_ptr<Scope> getGlobalScope() const;
	};

	using ProgramPtr = std::shared_ptr<Program>;
}
