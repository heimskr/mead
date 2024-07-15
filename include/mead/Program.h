#pragma once

#include <memory>

namespace mead {
	class Namespace;

	class Program {
		private:
			std::shared_ptr<Namespace> globalNamespace;

		public:
			Program();
	};
}
