#include "mead/ASTNode.h"

#include <cassert>

namespace mead {
	ASTNode::ASTNode():
		valid(false) {}

	ASTNode::ASTNode(NodeType type, Token token, std::weak_ptr<ASTNode> parent):
		type(type), token(std::move(token)), parent(std::move(parent)), valid(true) {}

	std::shared_ptr<ASTNode> ASTNode::reparent(std::weak_ptr<ASTNode> new_parent) {
		assert(parent.expired());
		parent = std::move(new_parent);
		auto self = shared_from_this();

		if (auto locked = parent.lock()) {
			locked->children.push_back(self);
		}

		return self;
	}
}
