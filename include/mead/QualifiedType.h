#pragma once

#include "mead/Type.h"

#include <vector>

namespace mead {
	class QualifiedType {
		private:
			std::vector<bool> pointerConsts;
			bool isConst = false;
			bool isReference = false;
			TypePtr type;

		public:
			QualifiedType();
			QualifiedType(std::vector<bool> pointer_consts, bool is_const, bool is_reference, TypePtr type);

			size_t pointerLevel() const;
	};
}
