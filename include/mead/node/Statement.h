#pragma once

#include "mead/ASTNode.h"

namespace mead {
	class Statement: public ASTNode {
		protected:
			using ASTNode::ASTNode;
	};
}