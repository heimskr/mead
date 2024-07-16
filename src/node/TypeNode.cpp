#include "mead/node/TypeNode.h"
#include "mead/Namespace.h"
#include "mead/Type.h"

namespace mead {
	TypeNode::TypeNode(Token token):
		ASTNode(NodeType::Type, std::move(token)) {}

	std::shared_ptr<Type> TypeNode::getType(const std::shared_ptr<Namespace> &ns) const {
		return {};
	}
}
