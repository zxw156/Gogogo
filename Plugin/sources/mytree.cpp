#include "mytree.h"
#include <iostream>

void MTree::init(MNode *root) { this->root = root; }

MNode* MTree::getRoot() { return this->root; }

void MTree::putChild(MNode *node, MNode *parent) {
	parent->children.push_back(node);
	node->Parent = parent;
}


void MTree::putChildren(vector<MNode *> nodes, MNode *parent) {
	for (int i = 0; i < nodes.size(); ++i) {
		putChild(nodes[i], parent);
	}
}

int MTree::getMaxDepth(MNode *root, vector<MNode*> nodes) {
	auto iResult = 0;

	return iResult;
}