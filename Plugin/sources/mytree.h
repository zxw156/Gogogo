#ifndef __MYTREE_H__
#define __MYTREE_H__

#include <vector>
#include "bookmark.h"
#include "wx/treectrl.h"

using namespace std;

typedef int T;
typedef struct MNode {
	const char* title;
	MyBookmark bookmark;
	vector<MNode*> children;
	MNode *Parent;
} MNode;

class MTree {
private:
	MNode *root;

public:
	void init(MNode *root);
	MNode *getRoot();
	void putChild(MNode* node, MNode* parent);
	void putChildren(vector<MNode*> nodes, MNode *parent);
	int getMaxDepth(MNode *root, vector<MNode*> nodes);
};

class MyTreeItemData : public wxTreeItemData
{
public:
	MyTreeItemData(const int& index, const MyBookmark& bookmark) : m_index(index), m_bookmark(bookmark) { }
	const int& GetIndex() const { return m_index; }
	const MyBookmark& GetBookmark() const { return m_bookmark; }
private:
	MyBookmark m_bookmark;
	int m_index;
};

#endif // !__MYTREE_H__
