#include "mead/ASTNode.h"

namespace mead {
	ASTNode::ASTNode(NodeType type, Token token, std::weak_ptr<ASTNode> parent):
		type(type), token(std::move(token)), parent(std::move(parent)) {}
}
