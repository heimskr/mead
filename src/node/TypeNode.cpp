#include "mead/node/TypeNode.h"
#include "mead/Logging.h"
#include "mead/Namespace.h"
#include "mead/Type.h"

namespace mead {
	TypeNode::TypeNode(Token token):
		ASTNode(NodeType::Type, std::move(token)) {}

	std::shared_ptr<Type> TypeNode::getType(const std::shared_ptr<Namespace> &ns) const {
		TypePtr type = ns->getType(token.value);

		if (empty())
			return type;

		for (const ASTNodePtr &child : children) {
			switch (child->type) {
				case NodeType::Pointer:
					type = std::make_shared<PointerType>(std::move(type));
					break;
				case NodeType::LReference:
					type = std::make_shared<LReferenceType>(std::move(type));
					break;
				case NodeType::Const:
					type = type->copy();
					type->setConst(true);
					break;
				default:
					WARN("{}???", child->type);
					break;
			}
		}

		return type;
	}
}
