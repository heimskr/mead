#pragma once

#include <memory>
#include <string>

namespace mead {
	class Value {
		protected:
			Value() = default;

		public:
			virtual ~Value() = default;

			virtual std::string toString() const;
	};

	using ValuePtr = std::shared_ptr<Value>;
}
