#pragma once

#include <string>

namespace mead {
	class Symbol {
		protected:
			std::string name;
			Symbol(std::string name);

		public:
			virtual ~Symbol() = default;
	};
}
