/*********************************************************************

 ADOBE SYSTEMS INCORPORATED
 Copyright (C) 2008 Adobe Systems Incorporated
 All rights reserved.

 NOTICE: Adobe permits you to use, modify, and distribute this file
 in accordance with the terms of the Adobe license agreement
 accompanying it. If you have received this file from a source other
 than Adobe, then your use, modification, or distribution of it
 requires the prior written permission of Adobe.

 -------------------------------------------------------------------*/
/** 
\file wxPlugin.cpp

  - This file implements the functionality of the wxPlugin.
*********************************************************************/


// Acrobat Headers.
#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#endif

#include "wx/app.h"
#include "wx/msgdlg.h"
#include "wx/artprov.h"
#include "wx/imaglist.h"
#include "wx/image.h"
#include "wxInit.h"
#include "dialogs.h"
#include "link_dialogs.h"

#include "PIMain.h"
#include "AVCalls.h"

#include <map>
#include <fstream>

/*-------------------------------------------------------
	Constants/Declarations
-------------------------------------------------------*/
bool show = true;
 
// this plug-in's name, you should specify your own unique name here.
const char* MyPluginExtensionName = "ADBE:wxPlugin";
typedef ACCB1 void ACCB2 (*menuCommand)(void *clientData);

/* A convenient function to add a menu item for your plugin.
** A policy in Acrobat 6 is to add menu items for developers' plugins under Advanced menu.
** If specify bUnderAcrobatSDKSubMenu as false, you directly add your menu item under Advanced menu.
** If specify bUnderAcrobatSDKSubMenu as true, you add the menu item under Acrobat SDK sub menu.
*/

ACCB1 ASBool ACCB2 PluginMenuItem(char* MyMenuItemTitle, char* MyMenuItemName);

/*-------------------------------------------------------
	Functions
-------------------------------------------------------*/

std::map<wxString, int> mapStyleFlag = {
	{ "规则", kPDBookmarkFontNormal },
	{ "粗体", kPDBookmarkFontBold },
	{ "斜体", kPDBookmarkFontItalic },
	{ "粗斜体", kPDBookmarkFontBoldItalic }
};

std::map<wxString, bool> mapOpenFlag = {
	{ "新建窗口", true },
	{ "现有窗口", false },
};

std::map<wxString, wxString> mapZoomFlag = {
	{ "适合页面", "Fit" },
	{ "实际大小", "XYZ" }, // 100%
	{ "适合宽度", "FitH" },
	{ "适合可见", "FitBH" },
	{ "承前缩放", "XYZ" },	// null null 0
	{ "缩放", "XYZ" },			// 
};

std::map<wxString, wxString> mapFitTypeGetString = {
	{ "FitH", "适合宽度" },
	{ "Fit", "适合页面" },
	{ "FitBH", "适合可见" },
};

std::map<int, wxString> mapHighlight = {
	{ 0, "N" },	// 无
	{ 1, "I" },	// 反色
	{ 2, "O" }, // 边框
	{ 3, "P" },	// 内陷
};

std::map<int, wxString> mapLineType = {
	{ 0, "S" },	// 实线
	{ 1, "D" },	// 虚线
	{ 2, "U" }, // 下划线
};

ACCB1 CosObj ACCB2 SetColor(CosDoc cosDoc, std::vector<float> color)
{
	CosObj cosColor = CosNewArray(cosDoc, false, 3);
	CosArrayPut(cosColor, 0, CosNewFloat(cosDoc, false, color[0]));
	CosArrayPut(cosColor, 1, CosNewFloat(cosDoc, false, color[1]));
	CosArrayPut(cosColor, 2, CosNewFloat(cosDoc, false, color[2]));
	return cosColor;
}

ACCB1 vector<float> ACCB2 GetColor(PDBookmark bookmark)
{
	vector<float> color;
	CosObj cosBookmark = PDBookmarkGetCosObj(bookmark);
	if (CosDictKnown(cosBookmark, ASAtomFromString("C"))) {
		CosObj cosColor = CosDictGet(cosBookmark, ASAtomFromString("C"));
		for (int i = 0; i < 3; i++) {
			float colorf = CosFloatValue(CosArrayGet(cosColor, i));
			color.push_back(colorf);
		}
	}
	return color;
}

ACCB1 CosObj ACCB2 SetDestination(CosDoc cosDoc, CosObj cosPage, wxString zoomType, float zoomRate)
{
	CosObj cosDest = CosNewArray(cosDoc, false, 5);
	CosArrayPut(cosDest, 0, cosPage);
	CosArrayPut(cosDest, 1, CosNewName(cosDoc, false, ASAtomFromString(mapZoomFlag[zoomType])));
	if(zoomType == "缩放")
		CosArrayPut(cosDest, 4, CosNewFloat(cosDoc, false, zoomRate));
	else if (zoomType == "承前缩放")
		CosArrayPut(cosDest, 4, CosNewFloat(cosDoc, false, 0));
	else if (zoomType == "实际大小")
		CosArrayPut(cosDest, 4, CosNewFloat(cosDoc, false, 1));
	return cosDest;
}

ACCB1 PDBookmark ACCB2 BookmarkFromIndexArray(PDBookmark rootBookmark, vector<int> indexes)
{
	PDBookmark childBookmark;
	for (int i = 0; i < indexes.size(); i++)
	{
		childBookmark = PDBookmarkGetFirstChild(rootBookmark);
		for (int j = 0; j < indexes[i]; j++)
		{
			childBookmark = PDBookmarkGetNext(childBookmark);
		}
		rootBookmark = childBookmark;
	}
	return childBookmark;
}

ACCB1 PDAction ACCB2 GetPDAction(PDDoc pdCurrentDoc, MyBookmarkAction action)
{
	int actionName = action.actionName;
	wxString filepath = action.filepath;
	int page = action.page - 1;
	wxString zoomType = action.zoomType;
	float zoomRate = action.zoomRate;
	wxString option = action.option;
	bool useNamedDest = action.useNamedDest;
	wxString destName = action.destName;

	CosDoc cosDoc = PDDocGetCosDoc(pdCurrentDoc);

	//Set Action
	// 0 None
	// 1 Open a view    打开页面
	// 2 Open a file    打开文档
	// 3 打开文档页面
	CosObj cosAction = CosNewDict(cosDoc, false, 4);
	switch (actionName) {
	case 1: {
		CosDictPutKeyString(cosAction, "S", CosNewName(cosDoc, false, ASAtomFromString("GoTo")));
		if (useNamedDest)
			CosDictPutKeyString(cosAction, "D", CosNewString(cosDoc, false, destName, strlen(destName)));
		else {
			PDPage pdPage = PDDocAcquirePage(pdCurrentDoc, page);
			CosObj cosDest = SetDestination(cosDoc, PDPageGetCosObj(pdPage), zoomType, zoomRate);
			CosDictPutKeyString(cosAction, "D", cosDest);
		}
		break;
	}
	case 2: {
		CosDictPutKeyString(cosAction, "S", CosNewName(cosDoc, false, ASAtomFromString("GoToR")));
		CosDictPutKeyString(cosAction, "F", CosNewString(cosDoc, false, filepath, strlen(filepath)));
		if (option != "用户首选项设置的窗口")
			CosDictPutKeyString(cosAction, "NewWindow", CosNewBoolean(cosDoc, false, mapOpenFlag[option]));
		break;
	}
	case 3: {
		CosDictPutKeyString(cosAction, "S", CosNewName(cosDoc, false, ASAtomFromString("GoToR")));
		CosDictPutKeyString(cosAction, "F", CosNewString(cosDoc, false, filepath, strlen(filepath)));
		if (useNamedDest)
			CosDictPutKeyString(cosAction, "D", CosNewName(cosDoc, false, ASAtomFromString(destName)));
		else {
			CosObj cosDest = SetDestination(cosDoc, CosNewInteger(cosDoc, false, page), zoomType, zoomRate);
			CosDictPutKeyString(cosAction, "D", cosDest);
		}
		if (option != "用户首选项设置的窗口")
			CosDictPutKeyString(cosAction, "NewWindow", CosNewBoolean(cosDoc, false, mapOpenFlag[option]));
		break;
	}
	default:
		break;
	}

	PDAction pdAction;
	if (actionName)
	{
		pdAction = PDActionFromCosObj(cosAction);
	}
	return pdAction;
}

ACCB1 void ACCB2 SetBookmark(PDDoc pdCurrentDoc, PDBookmark* childBookmark, MyBookmark bookmark)
{
	vector<float> color;
	int style = bookmark.style;

	CosObj cosBookmark = PDBookmarkGetCosObj(*childBookmark);
	CosDoc cosDoc = PDDocGetCosDoc(pdCurrentDoc);

	//Set Style
	CosDictPutKeyString(cosBookmark, "F", CosNewInteger(cosDoc, false, style));
	//Set Color
	for (int i = 0; i < 3; i++) {
		color.push_back(bookmark.color[i] / 255.0);
	}
	CosObj cosColor = SetColor(cosDoc, color);
	CosDictPutKeyString(cosBookmark, "C", cosColor);

	MyBookmarkAction action = bookmark.action;
	PDAction pdAction = GetPDAction(pdCurrentDoc, action);

	if (action.actionName)
	{
		PDBookmarkSetAction(*childBookmark, pdAction);
	}
}
ACCB1 void ACCB2 AddAction(PDDoc pdCurrentDoc , PDBookmark* childBookmark, MyBookmark bookmark)
{
	vector<float> color;
	int style = bookmark.style;

	MyBookmarkAction action = bookmark.action;
	int actionName = action.actionName;
	wxString filepath = action.filepath;
	int page = action.page - 1;
	wxString zoomType = action.zoomType;
	float zoomRate = action.zoomRate;
	wxString option = action.option;
	bool useNamedDest = action.useNamedDest;
	wxString destName = action.destName;
	CosObj cosBookmark = PDBookmarkGetCosObj(*childBookmark);
	CosDoc cosDoc = PDDocGetCosDoc(pdCurrentDoc);

	if (useNamedDest) {
	}

	//Set Style
	CosDictPutKeyString(cosBookmark, "F", CosNewInteger(cosDoc, false, style));
	//Set Color
	for (int i = 0; i < 3; i++) {
		color.push_back(bookmark.color[i] / 255.0);
	}
	CosObj cosColor = SetColor(cosDoc, color);
	CosDictPutKeyString(cosBookmark, "C", cosColor);

	//Set Action
	// 0 None
	// 1 Open a view    打开页面
	// 2 Open a file    打开文档
	// 3 打开文档页面
	CosObj cosAction = CosNewDict(cosDoc, false, 4);
	switch (actionName) {
	case 1: {
		CosDictPutKeyString(cosAction, "S", CosNewName(cosDoc, false, ASAtomFromString("GoTo")));
		if (useNamedDest)
			CosDictPutKeyString(cosAction, "D", CosNewString(cosDoc, false, destName, strlen(destName)));
		else {
			PDPage pdPage = PDDocAcquirePage(pdCurrentDoc, page);
			CosObj cosDest = SetDestination(cosDoc, PDPageGetCosObj(pdPage), zoomType, zoomRate);
			CosDictPutKeyString(cosAction, "D", cosDest);
		}
		break;
	}
	case 2: {
		CosDictPutKeyString(cosAction, "S", CosNewName(cosDoc, false, ASAtomFromString("GoToR")));
		CosDictPutKeyString(cosAction, "F", CosNewString(cosDoc, false, filepath, strlen(filepath)));
		if (option != "用户首选项设置的窗口")
			CosDictPutKeyString(cosAction, "NewWindow", CosNewBoolean(cosDoc, false, mapOpenFlag[option]));
		break;
	}
	case 3: {
		CosDictPutKeyString(cosAction, "S", CosNewName(cosDoc, false, ASAtomFromString("GoToR")));
		CosDictPutKeyString(cosAction, "F", CosNewString(cosDoc, false, filepath, strlen(filepath)));
		if (useNamedDest)
			CosDictPutKeyString(cosAction, "D", CosNewName(cosDoc, false, ASAtomFromString(destName)));
		else {
			CosObj cosDest = SetDestination(cosDoc, CosNewInteger(cosDoc, false, page), zoomType, zoomRate);
			CosDictPutKeyString(cosAction, "D", cosDest);
		}
		if (option != "用户首选项设置的窗口")
			CosDictPutKeyString(cosAction, "NewWindow", CosNewBoolean(cosDoc, false, mapOpenFlag[option]));
		break;
	}
	default:
		break;
	}
	if (actionName)
	{
		PDAction pdAction = PDActionFromCosObj(cosAction);
		PDBookmarkSetAction(*childBookmark, pdAction);
	}
}

ACCB1 void ACCB2 DeleteBookmark(MyBookmark bookmark)
{
	AVDoc avCurrentDoc = AVAppGetActiveDoc();
	PDDoc pdCurrentDoc = AVDocGetPDDoc(avCurrentDoc);

	//Get the root bookmark
	PDBookmark rootBookmark;
	rootBookmark = PDDocGetBookmarkRoot(pdCurrentDoc);
	if (PDBookmarkIsValid(rootBookmark))
	{
		PDBookmark childBookmark = BookmarkFromIndexArray(rootBookmark, bookmark.delIndexes);
		PDBookmarkDestroy(childBookmark);
	}
}

ACCB1 void ACCB2 EditBookmark(MyBookmark bookmark)
{
	AVDoc avCurrentDoc = AVAppGetActiveDoc();
	PDDoc pdCurrentDoc = AVDocGetPDDoc(avCurrentDoc);

	//Get the root bookmark
	PDBookmark rootBookmark;
	rootBookmark = PDDocGetBookmarkRoot(pdCurrentDoc);
	if (PDBookmarkIsValid(rootBookmark))
	{
		PDBookmark childBookmark = BookmarkFromIndexArray(rootBookmark, bookmark.delIndexes);
		// Edit
		ASText title = ASTextFromEncoded(bookmark.name, PDGetHostEncoding());
		PDBookmarkSetTitleASText(childBookmark, title);
		PDBookmarkRemoveAction(childBookmark);
		AddAction(pdCurrentDoc, &childBookmark, bookmark);
	}
}

ACCB1 void ACCB2 AddBookmark(MyBookmark bookmark, bookmarkState state)
{
	AVDoc avCurrentDoc = AVAppGetActiveDoc();
	AVPageView pageView = AVDocGetPageView(avCurrentDoc);
	PDDoc pdCurrentDoc = AVDocGetPDDoc(avCurrentDoc);

	//Get the root bookmark
	PDBookmark rootBookmark;
	PDBookmark childBookmark ;
	rootBookmark = PDDocGetBookmarkRoot(pdCurrentDoc);
	if (PDBookmarkIsValid(rootBookmark))
	{
		PDBookmark insertBookmark;
		ASText title = ASTextFromEncoded(bookmark.name, PDGetHostEncoding());
		if (state != Add)
			insertBookmark = BookmarkFromIndexArray(rootBookmark, bookmark.delIndexes);

		switch (state)
		{
		case Add: {
			//Add a child bookmark to the root bookmark
			childBookmark = PDBookmarkAddNewChildASText(rootBookmark, title);
			AddAction(pdCurrentDoc, &childBookmark, bookmark);
			break;
		}
		case InsertNextSibling: {
			childBookmark = PDBookmarkAddNewSiblingASText(insertBookmark, title);
			AddAction(pdCurrentDoc, &childBookmark, bookmark);
			break;
		}
		case InsertPrevSibling: {
			PDBookmark parentBookmark = PDBookmarkGetParent(insertBookmark);
			childBookmark = PDBookmarkAddNewChildASText(parentBookmark, title);
			AddAction(pdCurrentDoc, &childBookmark, bookmark);
			PDBookmarkUnlink(childBookmark);
			PDBookmarkAddPrev(insertBookmark, childBookmark);
			break;
		}
		case InsertChild:
			childBookmark = PDBookmarkAddNewChildASText(insertBookmark, title);
			AddAction(pdCurrentDoc, &childBookmark, bookmark);
			break;
		default:
			break;
		}
	}
}

ACCB1 void ACCB2 GetPDDestAttr(CosObj cosDest, int &page, wxString &zoomType, float &zoomRate, bool &useNameDest, wxString &destName)
{
	CosType destType = CosObjGetType(cosDest);
	if (destType == CosName) {
		useNameDest = true;
		destName = ASAtomGetString(CosNameValue(cosDest));
		return;
	}
	else if (destType == CosString) {
		useNameDest = true;
		ASTCount bytes;
		destName = CosStringValue(cosDest, &bytes);
		return;
	}

	if (!destType == CosArray) {
		AVAlertNote("Destination Type is not CosArray");
		return;
	}

	useNameDest = false;
	// Get page info
	CosObj cosPage = CosArrayGet(cosDest, 0);
	CosType cosType = CosObjGetType(cosPage);

	if (cosType == CosDict) {
		//if (CosDictKnown(cosPage, ASAtomFromString("Contents"))){
		//	CosObj cosPageContents = CosDictGet(cosPage, ASAtomFromString("Contens"));
		//	CosObj content = CosArrayGet(cosPageContents, 0);
		//	cosType = CosObjGetType(content);
		//}
		page = 1;
	}
	else if (cosType == CosInteger) {
		page = CosIntegerValue(cosPage) + 1;
	}
	// Get fit type
	CosObj cosFitType = CosArrayGet(cosDest, 1);
	ASAtom fitType = CosNameValue(cosFitType);
	//Get zoom
	zoomRate = 1;
	if (fitType == ASAtomFromString("XYZ")) {
		CosObj cosZoomRate = CosArrayGet(cosDest, 4);
		CosType zoomRateType = CosObjGetType(cosZoomRate);
		if (zoomRateType == CosFixed) {
			ASFixed zoom = CosFixedValue(cosZoomRate);
			zoomRate = ASFixedToFloat(zoom);
			if (zoomRate == 0)
				zoomType = "承前缩放";
			else if (zoomRate == 1)
				zoomType = "实际大小";
			else
				zoomType = "缩放";
		}
		else if (zoomRateType == CosInteger) {
			int zoom = CosIntegerValue(cosZoomRate);
			zoomRate = (float)zoom;
			if (zoomRate == 0)
				zoomType = "承前缩放";
			else if (zoomRate == 1)
				zoomType = "实际大小";
			else
				zoomType = "缩放";
		}
		else if (zoomRateType == CosNull)
			zoomType = "承前缩放";
		else
			AVAlertNote(wxString::Format("zoom rate type: %i", zoomRateType));
	}
	else {
		zoomType = mapFitTypeGetString[ASAtomGetString(fitType)];
	}
}

ACCB1 MyBookmarkAction ACCB2 GetAction(PDAction pdAction)
{
	MyBookmarkAction myAction; // actionName, filepath, page, zoomType, zoomRate, option
	if (!PDActionIsValid(pdAction)) {
		return myAction;
	}
	ASAtom subtype = PDActionGetSubtype(pdAction);
	CosObj cosAction = PDActionGetCosObj(pdAction);
	CosType cosActionType = CosObjGetType(cosAction);

	int page;
	wxString zoomType;
	float zoomRate;
	bool useNamedDest;
	wxString destName;
	if (subtype == ASAtomFromString("GoTo")) {
		myAction.actionName = 1;
		if (cosActionType == CosString) {
			ASTCount bytes;
			useNamedDest = true;
			destName = CosStringValue(cosAction, &bytes);
		}
		else if (cosActionType == CosDict) {
			CosObj cosDest;
			if (CosDictKnown(cosAction, ASAtomFromString("D")))
				cosDest = CosDictGet(cosAction, ASAtomFromString("D"));
			else {
				PDViewDestination pdViewDest = PDActionGetDest(pdAction);
				cosDest = PDViewDestGetCosObj(pdViewDest);
			}
			GetPDDestAttr(cosDest, page, zoomType, zoomRate, useNamedDest, destName);
		}
	}
	else if (subtype == ASAtomFromString("GoToR")) {
		if (!CosDictKnown(cosAction, ASAtomFromString("F"))) {
			//AVAlertNote(wxString::Format("Bookmark %s : GoToR action has no 'F'.", title));
		}
		else {
			CosObj cosFilepath = CosDictGet(cosAction, ASAtomFromString("F"));
			if (!CosObjGetType(cosFilepath) == CosString)
				AVAlertNote(wxString::Format("file path cos type: %i", CosObjGetType(cosFilepath)));
			else {
				char *buffer;
				ASTCount bytes;
				buffer = CosStringValue(cosFilepath, &bytes);
				myAction.filepath = buffer;
			}
		}

		if (CosDictKnown(cosAction, ASAtomFromString("D"))) {
			myAction.actionName = 3;
			CosObj cosDest = CosDictGet(cosAction, ASAtomFromString("D"));
			GetPDDestAttr(cosDest, page, zoomType, zoomRate, useNamedDest, destName);
		}
		else
			myAction.actionName = 2;		// open file

		// get window option
		if (CosDictKnown(cosAction, ASAtomFromString("NewWindow"))) {
			CosObj cosWindowOption = CosDictGet(cosAction, ASAtomFromString("NewWindow"));
			bool windowOption = CosBooleanValue(cosWindowOption);
			if (windowOption)
				myAction.option = "新建窗口";
			else
				myAction.option = "现有窗口";
		}
		else
			myAction.option = "用户首选项设置的窗口";
	}
	myAction.page = page;
	myAction.zoomType = zoomType;
	myAction.zoomRate = zoomRate;
	myAction.useNamedDest = useNamedDest;
	myAction.destName = destName;
	
	//AVAlertNote(wxString::Format("actionname:%i, page:%i, filepath:%s", myAction.actionName, myAction.page, myAction.filepath));

	return myAction;
}

ACCB1 void ACCB2 VisitAllBookmarks(PDBookmark bookmark, MTree* tree, MNode* parentNode)
{
	PDBookmark treeBookmark;
	
	//Ensure that the bookmark is valid
	if (!PDBookmarkIsValid(bookmark))
		return;

	//Get the title of the bookmark
	ASText title = ASTextNew();
	PDBookmarkGetTitleASText(bookmark, title);
	const char* bmBuf = ASTextGetEncoded(title, PDGetHostEncoding());

	//Get action
	PDAction pdAction = PDBookmarkGetAction(bookmark);
	MyBookmarkAction myAction = GetAction(pdAction);

	//Get the flags / style of the bookmark
	ASInt32 flag = PDBookmarkGetFlags(bookmark);

	MNode *childNode = new MNode;
	childNode->title = bmBuf;
	childNode->bookmark.name = bmBuf;
	vector<float> color = GetColor(bookmark);
	for (int i = 0; i < color.size(); i++)
		childNode->bookmark.color[i] = color[i] * 255.0;
	childNode->bookmark.action = myAction;
	childNode->bookmark.pdAction = pdAction;
	childNode->bookmark.style = flag;
	tree->putChild(childNode, parentNode);

	//Determine if the current bookmark has children bookmark
	if (PDBookmarkHasChildren(bookmark))
	{
		//Get the first child of the bookmark
		treeBookmark = PDBookmarkGetFirstChild(bookmark);
		while (PDBookmarkIsValid(treeBookmark)) {
			VisitAllBookmarks(treeBookmark, tree, childNode);
			treeBookmark = PDBookmarkGetNext(treeBookmark);
		}
	}
}

wxArrayString names;
ACCB1 ASBool ACCB2 nameTreeEnumerator(CosObj obj, CosObj value, void *clientData)
{
	ASTCount bytes;
	names.Add(CosStringValue(obj, &bytes));
	//AVAlertNote(CosStringValue(obj, &bytes));

	return true;
}

ACCB1 wxArrayString ACCB2 GetOriginNamedDest(PDDoc doc)
{
	names.Empty();
	CosObjEnumProc cosObjProc = NULL;
	cosObjProc = ASCallbackCreateProto(CosObjEnumProc, &nameTreeEnumerator);
	PDNameTree namesTree = PDDocGetNameTree(doc, ASAtomFromString("Names"));
	PDNameTree destsTree = PDDocGetNameTree(doc, ASAtomFromString("Dests"));
	if (PDNameTreeIsValid(namesTree)) {
		PDNameTreeEnum(namesTree, cosObjProc, NULL);
	}
	else if (PDNameTreeIsValid(destsTree)) {
		PDNameTreeEnum(destsTree, cosObjProc, NULL);
	}
	return names;
}

ACCB1 void ACCB2 GetPDEContent(PDEContent pdeContent) {
	ASInt32 elementNum = PDEContentGetNumElems(pdeContent);
	AVAlertNote(wxString::Format("element number:%i", elementNum));
	//ofstream outfile("C:\\Users\\#\\Desktop\\textcontent.txt", ios::app);
	for (int i = 0; i < elementNum; i++) {
		PDEElement pdeElement;
		pdeElement = PDEContentGetElem(pdeContent, i);
		AVAlertNote("get element");
		ASInt32 type = PDEObjectGetType((PDEObject)pdeElement);
		switch (type) {
		case kPDEPlace:
			AVAlertNote("kpdeplace");
			break;
		case kPDEText: {
			AVAlertNote("kpdetext");
			int runNum = PDETextGetNumRuns((PDEText)pdeElement);
			AVAlertNote(wxString::Format("run num:%i", runNum));
			for (int j = 0; j < runNum; j++) {
				ASInt32 len = PDETextGetText((PDEText)pdeElement, kPDETextRun, j, NULL);
				ASUns8* buffer = new ASUns8[len];
				PDETextGetText((PDEText)pdeElement, kPDETextRun, j, buffer);
				//outfile << buffer;
			}
			break;
		}
		case kPDEContainer: {
			AVAlertNote("kpdecontainer");
			PDEContent content = PDEContainerGetContent((PDEContainer)pdeElement);
			GetPDEContent(content);
			break;
		}
		case kPDEPath:
			AVAlertNote("kpdepath");
			break;
		default:
			AVAlertNote(wxString::Format("kpde type:%i", type));
			break;
		}
	}
	//outfile.close();
}

ACCB1 MTree* ACCB2 GetOriginBookmarks(PDDoc doc)
{
	PDBookmark theroot, childBookmark;
	theroot = PDDocGetBookmarkRoot(doc);

	MNode *node = new MNode;
	MTree *tree = new MTree;
	node->Parent = nullptr;
	node->title = "Bookmark";
	tree->init(node);

	if (PDBookmarkHasChildren(theroot))
	{
		childBookmark = PDBookmarkGetFirstChild(theroot);
		while (PDBookmarkIsValid(childBookmark)) {
			VisitAllBookmarks(childBookmark, tree, node);
			childBookmark = PDBookmarkGetNext(childBookmark);
		}
	}

	return tree;
}

ACCB1 void ACCB2 ProcessTreeRootElement(PDSElement element)
{
	//unsigned char* buffer;
	//PDSElementGetTitle(element, buffer);
	//AVAlertNote((const char*)buffer);
	ASAtom type = PDSElementGetType(element);
	if (type == ASAtomFromString("TOC")) {
		AVAlertNote("TOC");
	}
	else if (type == ASAtomFromString("TOCI")) {
		AVAlertNote("TOCI");
	}
	else {
		AVAlertNote(ASAtomGetString(type));
		//return;
	}
	ASInt32 kidNum = PDSElementGetNumKids(element);
	AVAlertNote(wxString::Format("kid num:%i", kidNum));
	CosObj kidObj;
	void* pointerKid;
	CosObj cosPage;
	for (int i = 0; i < kidNum; i++) {
		ASAtom kidType = PDSElementGetKid(element, i, &kidObj, &pointerKid, &cosPage);
		if (kidType = ASAtomFromString("StructElem")) {
			//AVAlertNote("structelem");
			ProcessTreeRootElement(kidObj);
		}
	}
}

ACCB1 ASBool ACCB2 pdTextSelectTextEnumerator(void *procObj, PDFont font, ASFixed size, PDColorValue color, char *text, ASInt32 textLen)
{
	AVAlertNote(text);
	return true;
}

int textNum = 0;
int lastTextNum = 0;
ACCB1 void ACCB2 ProcessContent(PDEContent &pdeContent, int start, int end) {
	ASInt32 eleNum = PDEContentGetNumElems(pdeContent);
	for (int i = 0; i < eleNum; i++) {
		PDEElement pdeElement = PDEContentGetElem(pdeContent, i);
		ASInt32 type = PDEObjectGetType((PDEObject)pdeElement);
		switch (type) {
		case kPDEText: {
			//AVAlertNote("text");
			PDEText textObject = (PDEText)pdeElement;
			ASInt32 numChars = PDETextGetNumChars(textObject);
			ASInt32 numTextRun = PDETextGetNumRuns(textObject);
			ASInt32 startRunIndex = 0;
			ASInt32 endRunIndex = numTextRun;
			ASInt32 nextTextNum = textNum + numChars;
			ASInt32 startChar = start - textNum;
			ASInt32 endChar = end - textNum;
			//AVAlertNote(wxString::Format("%d, %d", startRunIndex, endRunIndex));
			AVAlertNote(wxString::Format("textNum:%d, nextTextNum:%d, start:%d, end:%d", textNum, nextTextNum, start, end));
			if (textNum < start) {
				if (nextTextNum <= start) {
					textNum = nextTextNum;
					break;
				}
				else if (nextTextNum <= end) {
					startRunIndex = PDETextGetRunForChar(textObject, startChar);
				}
				else {
					startRunIndex = PDETextGetRunForChar(textObject, startChar);
					endRunIndex = PDETextGetRunForChar(textObject, endChar);
				}
			}
			else if (textNum <= end) {
				if (nextTextNum <= end) {
					
				}
				else {
					endRunIndex = PDETextGetRunForChar(textObject, endChar);
				}
			}
			else {
				textNum = nextTextNum;
				break;
			}

			//AVAlertNote(wxString::Format("%d, %d", startRunIndex, endRunIndex));

			if (textNum < start) {
				PDETextSplitRunAt(textObject, startChar - 1);
				startRunIndex += 1;
				endRunIndex += 1;
				AVAlertNote(wxString::Format("split start at %d", startChar - 1));
			}
			if (nextTextNum > end + 1) {
				PDETextSplitRunAt(textObject, endChar);
				AVAlertNote(wxString::Format("split end at %d", endChar));
			}

			//AVAlertNote(wxString::Format("%d, %d", startRunIndex, endRunIndex));
			for (int j = startRunIndex; j <= endRunIndex; j++) {
				//ASInt32 numChars = PDETextRunGetNumChars(textObject, j);
				//ASInt32 offset = PDETextRunGetCharOffset(textObject, j);

				PDEGraphicState stateOfRun;
				PDETextGetGState(textObject, kPDETextRun, j, &stateOfRun, sizeof(PDEGraphicState));
				PDEColorSpace colourSpace = PDEColorSpaceCreateFromName(ASAtomFromString("DeviceRGB"));
				PDEColorValue colourValue;
				colourValue.color[0] = FloatToASFixed(1);
				colourValue.color[1] = 0;
				colourValue.color[2] = 0;
				colourValue.colorObj = NULL;
				colourValue.colorObj2 = NULL;
				PDEColorSpec colourSpec;
				colourSpec.space = colourSpace;
				colourSpec.value = colourValue;
				stateOfRun.strokeColorSpec = stateOfRun.fillColorSpec = colourSpec;
				PDETextRunSetGState(textObject, j, &stateOfRun, sizeof(stateOfRun));
			}
			textNum = nextTextNum;
			break;
		}
		case kPDEContainer: {
			//AVAlertNote("container");
			PDEContainer containerObject = (PDEContainer)pdeElement;
			PDEContent content = PDEContainerGetContent(containerObject);
			ProcessContent(content, start, end);
			break;
		}
		default:
			//AVAlertNote(wxString::Format("type:%d", type));
			break;
		}
	}
}

ACCB1 void ACCB2 AddLinkAnnotation(MyLinkStyle style, MyLinkDest dest)
{
	int highlight = style.highlight;
	int linkType = style.linkType;
	int lineWidth = style.lineWidth;
	int lineStyle = style.lineStyle;
	float red = style.linkColor[0] / 255.0;
	float green = style.linkColor[1] / 255.0;
	float blue = style.linkColor[2] / 255.0;

	AVDoc avDoc = AVAppGetActiveDoc();
	AVPageView avPageView = AVDocGetPageView(avDoc);
	PDDoc pdDoc = AVDocGetPDDoc(avDoc);
	CosDoc cosDoc = PDDocGetCosDoc(pdDoc);
	ASAtom selectionType = AVDocGetSelectionType(avDoc);
	if (selectionType == ASAtomFromString("Text")) {
		PDTextSelect textSelect = (PDTextSelect)AVDocGetSelection(avDoc);
		ASInt32 pageNum = PDTextSelectGetPage(textSelect);
		PDPage pdPage = PDDocAcquirePage(pdDoc, pageNum);

		/* Add link annotation*/
		PDAnnot annot, linkAnnot;
		ASFixedRect fr;
		PDTextSelectGetBoundingRect(textSelect, &fr);
		annot = PDPageCreateAnnot(pdPage, ASAtomFromString("Link"), &fr);
		linkAnnot = CastToPDLinkAnnot(annot);

		CosObj annotObj = PDAnnotGetCosObj(linkAnnot);
		// highlight
		CosDictPutKeyString(annotObj, "H", CosNewNameFromString(cosDoc, false, mapHighlight[highlight]));
		// border style
		CosObj bsObj = CosNewDict(cosDoc, false, 3);
		if (linkType) {	// 不可见矩形
			CosDictPutKeyString(bsObj, "W", CosNewInteger(cosDoc, false, 0));
			CosDictPutKeyString(annotObj, "BS", bsObj);
		}
		else {
			CosDictPutKeyString(bsObj, "W", CosNewInteger(cosDoc, false, lineWidth + 1));
			CosDictPutKeyString(bsObj, "S", CosNewNameFromString(cosDoc, false, mapLineType[lineStyle]));
			if (mapLineType[lineStyle] == "D") {
				CosObj dashObj = CosNewArray(cosDoc, false, 1);
				CosArrayPut(dashObj, 0, CosNewInteger(cosDoc, false, 3));
				CosDictPutKeyString(bsObj, "D", dashObj);
			}
			CosDictPutKeyString(annotObj, "BS", bsObj);
			// color
			CosObj colorObj = CosNewArray(cosDoc, false, 3);
			CosArrayPut(colorObj, 0, CosNewFloat(cosDoc, false, red));
			CosArrayPut(colorObj, 1, CosNewFloat(cosDoc, false, green));
			CosArrayPut(colorObj, 2, CosNewFloat(cosDoc, false, blue));
			CosDictPutKeyString(annotObj, "C", colorObj);
		}

		/*Add Action*/
		PDAction action;
		if (dest.useBookmark) {
			action = dest.bookmarkAction;
			PDLinkAnnotSetAction(linkAnnot, action);
		}
		else {
			action = GetPDAction(pdDoc, dest.action);
			PDLinkAnnotSetAction(linkAnnot, action);
		}

		PDPageAddAnnot(pdPage, -2, linkAnnot);

		/*Deal with select text*/
		//PDTextSelectRange range;
		//PDTextSelectGetRange(textSelect, 0, range);
		//ASInt32 startSelect = range->start;
		//ASInt32 endSelect = range->end;
		//ASInt32 ofsStart = range->ofsStart;
		//ASInt32 ofsEnd = range->ofsEnd;

		//PDWordFinderConfigRec pConfig;
		//memset(&pConfig, 0, sizeof(PDWordFinderConfigRec));
		//pConfig.recSize = sizeof(PDWordFinderConfigRec);
		////pConfig.ignoreCharGaps = true;
		////pConfig.ignoreLineGaps = true;
		//pConfig.noAnnots = true;
		//pConfig.noEncodingGuess = true;
		//PDWordFinder pdWordFinder = PDDocCreateWordFinderEx(pdDoc, WF_LATEST_VERSION, false, &pConfig);

		//ASInt32 numWords;
		//PDWord wordList;
		//PDWordFinderAcquireWordList(pdWordFinder, pageNum, &wordList, NULL, NULL, &numWords);

		//PDWord startWord = PDWordFinderGetNthWord(pdWordFinder, startSelect);
		//PDWord endWord = PDWordFinderGetNthWord(pdWordFinder, endSelect);
		//ASInt32 startWordOffset = PDWordGetCharOffset(startWord);
		//ASInt32 endWordOffset = PDWordGetCharOffset(endWord);

		//PDWordFinderDestroy(pdWordFinder);

		//textNum = 0;
		//int start = startWordOffset + ofsStart / 2;
		//int end = endWordOffset + ofsEnd / 2;
		//PDEContent pdeContent = PDPageAcquirePDEContent(pdPage, gExtensionID);
		//AVAlertNote(wxString::Format("%d, %d", start, end));

		//ProcessContent(pdeContent, start, end);

		//PDPageSetPDEContent(pdPage, gExtensionID);
		//PDPageNotifyContentsDidChange(pdPage);
		//PDPageReleasePDEContent(pdPage, gExtensionID);

		//PDTextSelectEnumTextProc pdTextSelectTextProc = NULL;
		//pdTextSelectTextProc = ASCallbackCreateProto(PDTextSelectEnumTextProc, &pdTextSelectTextEnumerator);
		//PDTextSelectEnumText(textSelect, pdTextSelectTextProc, NULL);
	}
}

ACCB1 void ACCB2 MenuOnAddLinkAnnotation(void *clientData)
{
	HWND CapturehWnd = NULL;

	CapturehWnd = GetCapture();
	if (CapturehWnd != NULL)
	{
		ReleaseCapture();
	}

	AVDoc avDoc = AVAppGetActiveDoc();
	ASAtom selectionType = AVDocGetSelectionType(avDoc);
	if (selectionType == ASAtomNull) {
		AVAlertNote("请选取要添加超链接的文本");
		return;
	}

	HWND hParent = WinAppGetModalParent(avDoc);

	wxDialog *win = new wxDialog();
	win->SetHWND(hParent);

	AddLinkAnnotationDialog* dlg = new AddLinkAnnotationDialog(win);
	AVWindow dlgWindow = AVWindowNewFromPlatformThing(
		AVWLmodal, NULL, NULL, gExtensionID, reinterpret_cast<AVPlatformWindowRef>(dlg->GetHWND()));

	AVAppBeginModal(dlgWindow);
	dlg->Centre();
	int returnCode = dlg->ShowModal();

	if (returnCode == wxID_OK)
	{
		// process
		MyLinkAnnotation link = dlg->getLinkAnnotation();
		MyLinkStyle style = link.style;
		MyLinkDest dest = link.dest;
		AddLinkAnnotation(style, dest);
	}
	AVAppEndModal();

	BringWindowToTop(hParent);
	AVWindowDestroy(dlgWindow);

	dlg->Destroy();
	delete dlg;
}

/**	
	Open a modal dialog
*/ 
ACCB1 void ACCB2 MenuOnEditBookmark(void *clientData)
{
	HWND CapturehWnd=NULL, hParent=NULL;

	CapturehWnd = GetCapture();
    if ( CapturehWnd != NULL )
    {
        ReleaseCapture();
    }

	hParent = WinAppGetModalParent(AVAppGetActiveDoc());

	AVDoc avDoc = AVAppGetActiveDoc();
	PDDoc pdDoc = AVDocGetPDDoc(avDoc);
	MTree* tree = GetOriginBookmarks(pdDoc);
	//GetOriginNamedDest(pdDoc);

	wxDialog *win = new wxDialog();
    win->SetHWND(hParent);
    
	BookmarkManagerDialog* dlg = new BookmarkManagerDialog(win);
	dlg->SetOriginBookmarks(tree);
	//dlg->SetOriginNamedDests(names);
	AVWindow dlgWindow = AVWindowNewFromPlatformThing(
		AVWLmodal, NULL, NULL, gExtensionID, reinterpret_cast<AVPlatformWindowRef>(dlg->GetHWND()));

	AVAppBeginModal(dlgWindow);
	dlg->Centre();
	int returnCode = dlg->ShowModal();

	if (returnCode == wxID_OK)
	{
		std::vector<MyBookmark> bookmarks = dlg->GetBookmarks();
		for (int i = 0; i < bookmarks.size(); i++)
		{
			if (bookmarks[i].state == Add)
				AddBookmark(bookmarks[i], Add);
			else if (bookmarks[i].state == InsertNextSibling)
				AddBookmark(bookmarks[i], InsertNextSibling);
			else if (bookmarks[i].state == InsertPrevSibling)
				AddBookmark(bookmarks[i], InsertPrevSibling);
			else if (bookmarks[i].state == InsertChild)
				AddBookmark(bookmarks[i], InsertChild);
			else if (bookmarks[i].state == Delete)
				DeleteBookmark(bookmarks[i]);
			else if (bookmarks[i].state == Edit)
				EditBookmark(bookmarks[i]);
		}
	}
    AVAppEndModal();

	BringWindowToTop(hParent);
	AVWindowDestroy(dlgWindow);

	dlg->Destroy();
	delete dlg;
}

/* MyPluginSetmenu
** ------------------------------------------------------
**
** Function to set up menu for the plugin.
** It calls a convenient function PluginMenuItem.
** Return true if successful, false if failed.
*/
ACCB1 ASBool ACCB2 MyPluginSetmenu()
{
	// Add a new menu item under Acrobat SDK submenu.
	// The new menu item name is "ADBE:BasicPluginMenu", title is "Basic Plugin".
	// Of course, you can change it to your own.
	return PluginMenuItem("Bookmark", "ADBE:BookmarkMenu"); 
}


/* MyPluginIsEnabled
** ------------------------------------------------------
** Function to control if a menu item should be enabled.
** Return true to enable it, false not to enable it.
*/
ACCB1 ASBool ACCB2 MenuItemIsEnabledByAVDoc(void *clientData)
{
	AVDoc avDoc = AVAppGetActiveDoc();
	if (avDoc == NULL)
		return false;
	else
		return true;
}

ACCB1 ASBool ACCB2 MenuItemIsEnabled(void *clientData)
{
	return true;
}