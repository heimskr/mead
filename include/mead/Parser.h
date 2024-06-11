#pragma once

#include <memory>
#include <span>
#include <string>
#include <vector>

namespace mead {
	class ASTNode;

	class Parser {
		private:
			std::vector<std::shared_ptr<ASTNode>> astNodes;

		public:
			Parser();
	};
}
