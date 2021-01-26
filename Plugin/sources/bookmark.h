#ifndef __BOOKMARK_H__
#define __BOOKMARK_H__

#include "PDExpT.h"
#include "wx/dialog.h"
#include <vector>

typedef struct MyBookmarkAction
{
	int actionName = 0;
	wxString filepath;
	int page;
	wxString zoomType;
	float zoomRate;
	wxString option;
	bool useNamedDest = false;
	wxString destName;
}MyBookmarkAction;

enum bookmarkState
{
	Nothing,
	Add,
	InsertNextSibling,
	InsertPrevSibling,
	InsertChild,
	Edit,
	Delete,
};

typedef struct MyBookmark
{
	bookmarkState state = Nothing;		// 0-Nothing, 1-Add, 2-Edit, 3-Delete
										// Add / Edit bookmark used
	wxString name;
	int color[3] = { 0 };
	int style;	
	MyBookmarkAction action;
	PDAction pdAction;
	// Delete / Edit bookmark used
	std::vector<int> delIndexes;
}MyBookmark;

#endif // __BOOKMARK_H__
