#include "mead/BasicBlock.h"

namespace mead {
	BasicBlock::BasicBlock(Function &parent):
		parent(&parent) {}

	void BasicBlock::connectTo(BasicBlock &other) {
		out.insert(other.weak_from_this());
		other.in.insert(weak_from_this());
	}

	void BasicBlock::disconnect(BasicBlock &other) {
		auto weak_other = other.weak_from_this();
		in.erase(weak_other);
		out.erase(weak_other);

		auto weak_self = weak_from_this();
		other.in.erase(weak_self);
		other.out.erase(weak_self);
	}
}
