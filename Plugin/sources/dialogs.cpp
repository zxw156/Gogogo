/*********************************************************************

 ADOBE SYSTEMS INCORPORATED
 Copyright (C) 2008 Adobe Systems Incorporated
 All rights reserved.

 NOTICE: Adobe permits you to use, modify, and distribute this file
 in accordance with the terms of the Adobe license agreement
 accompanying it. If you have received this file from a source other
 than Adobe, then your use, modification, or distribution of it
 requires the prior written permission of Adobe.

 ---------------------------------------------------------------------

 dialogs.cpp

 - Simple dialog code for wxWidgets.

*********************************************************************/
#include "wx/app.h"
#include "wx/statbox.h"
#include "wx/listbox.h"
#include "wx/choice.h"
#include "wx/stattext.h"
#include "wx/textctrl.h"
#include "wx/sizer.h"
#include "wx/filedlg.h"
#include "wx/checkbox.h"
#include "wx/msgdlg.h"
#include "wx/textdlg.h"
#include "wxInit.h"

#include "dialogs.h"
#include <regex>

#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#endif
ACCB1 wxArrayString ACCB2 GetOriginNamedDest(PDDoc doc);
ACCB1 void ACCB2 GetTOC(PDDoc doc, int pageNum);

// ----------------------------------------------------------------------------
// BookmarkManagerDialog
// ----------------------------------------------------------------------------
BookmarkManagerDialog::BookmarkManagerDialog(wxWindow *parent)
	: wxDialog(parent, wxID_ANY, wxString(_T("编辑书签")))
{
	this->SetWindowStyleFlag(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);
	wxStaticBoxSizer *sizerBm = new wxStaticBoxSizer(wxVERTICAL, this, "分组框");

	m_tree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(400, 600), wxTR_HIDE_ROOT | wxTR_DEFAULT_STYLE);
	m_rootId = m_tree->AddRoot("书签");

	wxBoxSizer *sizerBtn = new wxBoxSizer(wxVERTICAL);
	m_btnAdd = new wxButton(this, wxID_ANY, "新增");
	m_btnEdit = new wxButton(this, wxID_ANY, "修改");
	m_btnRemove = new wxButton(this, wxID_ANY, "删除");
	sizerBtn->Add(m_btnAdd, 0, wxALIGN_CENTER | wxALL, 5);
	sizerBtn->Add(m_btnEdit, 0, wxALIGN_CENTER | wxALL, 5);
	sizerBtn->Add(m_btnRemove, 0, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer *sizerStaticBox = new wxBoxSizer(wxHORIZONTAL);
	sizerStaticBox->Add(m_tree, 0, wxALIGN_CENTER | wxALL, 5);
	sizerStaticBox->Add(sizerBtn, 0, wxALIGN_CENTER | wxALL, 5);

	sizerBm->Add(sizerStaticBox, wxALIGN_CENTER | wxALL, 10);

	wxBoxSizer *sizerBtnRight = new wxBoxSizer(wxVERTICAL);
	wxButton *btnOk = new wxButton(this, wxID_OK, "确定");
	wxButton *btnCancel = new wxButton(this, wxID_CANCEL, "取消");
	m_btnImport = new wxButton(this, wxID_ANY, "导入");
	wxStaticText *textInsertOption = new wxStaticText(this, wxID_ANY, "插入选项");
	wxArrayString stringInserOption;
	stringInserOption.Add("选中书签的下面");
	stringInserOption.Add("选中书签的上面");
	stringInserOption.Add("插入子节点");
	m_choiceInsertOption = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringInserOption);
	m_choiceInsertOption->SetSelection(0);
	sizerBtnRight->Add(btnOk, 0, wxALIGN_CENTER | wxALL, 5);
	sizerBtnRight->Add(btnCancel, 0, wxALIGN_CENTER | wxALL, 5);
	sizerBtnRight->Add(m_btnImport, 0, wxALIGN_CENTER | wxALL, 5);
	sizerBtnRight->Add(textInsertOption, 0, wxALIGN_CENTER | wxALL, 5);
	sizerBtnRight->Add(m_choiceInsertOption, 0, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer *sizerTop = new wxBoxSizer(wxHORIZONTAL);
	sizerTop->Add(sizerBm, 0, wxALIGN_CENTER | wxALL, 5);
	sizerTop->Add(sizerBtnRight, 0, wxALIGN_CENTER | wxALL, 5);

	SetSizer(sizerTop);

	sizerTop->SetSizeHints(this);
	sizerTop->Fit(this);

	m_btnAdd->SetFocus();
	m_btnAdd->SetDefault();
}

vector<int> BookmarkManagerDialog::ItemIdToIndexArray(wxTreeItemId itemId)
{
	vector<int> indexes;
	while (itemId.IsOk() && itemId != m_rootId) {
		int index = 0;
		wxTreeItemId prevId = m_tree->GetPrevSibling(itemId);
		while (prevId.IsOk()) {
			prevId = m_tree->GetPrevSibling(prevId);
			index++;
		}
		indexes.insert(indexes.begin(), index);
		itemId = m_tree->GetItemParent(itemId);
	}
	return indexes;
}

void BookmarkManagerDialog::OnButtonANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_btnAdd)
	{
		AddBookmarkDialog *dlg = new AddBookmarkDialog(this, MyBookmark());
		wxTreeItemId addItemId;
		int returnCode = dlg->ShowModal();
		if (returnCode == wxID_OK)
		{
			MyBookmark bookmark = dlg->GetBookmark();
			wxTreeItemId itemId = m_tree->GetSelection();
			int index = m_bookmarks.size();
			if (itemId.IsOk()) {
				bookmark.delIndexes = ItemIdToIndexArray(itemId);
				wxTreeItemId parentId = m_tree->GetItemParent(itemId);
				int insertOption = m_choiceInsertOption->GetSelection();
				switch (insertOption)
				{
				case 0:
					bookmark.state = InsertNextSibling;
					addItemId = m_tree->InsertItem(parentId, itemId, bookmark.name, -1, -1, new MyTreeItemData(index, bookmark));
					break;
				case 1: {
					bookmark.state = InsertPrevSibling;
					wxTreeItemId prevId = m_tree->GetPrevSibling(itemId);
					if (prevId.IsOk())
						addItemId = m_tree->InsertItem(parentId, prevId, bookmark.name, -1, -1, new MyTreeItemData(index, bookmark));
					else
						addItemId = m_tree->InsertItem(parentId, 0, bookmark.name, -1, -1, new MyTreeItemData(index, bookmark));
					break;
				}
				case 2:
					bookmark.state = InsertChild;
					addItemId = m_tree->AppendItem(itemId, bookmark.name, -1, -1, new MyTreeItemData(index, bookmark));
					break;
				default:
					break;
				}
			}
			else {
				bookmark.state = Add;
				wxTreeItemId rootId = m_tree->GetRootItem();
				addItemId = m_tree->AppendItem(rootId, bookmark.name, -1, -1, new MyTreeItemData(index, bookmark));
			}
			m_bookmarks.push_back(bookmark);
			m_tree->SetItemTextColour(addItemId, wxColor(bookmark.color[0], bookmark.color[1], bookmark.color[2]));
			wxFont font =  m_tree->GetItemFont(addItemId);
			if (bookmark.style == 1)
				font.SetStyle(wxFONTSTYLE_ITALIC);
			else if (bookmark.style == 2)
				font.SetWeight(wxFONTWEIGHT_BOLD);
			else if (bookmark.style == 3) {
				font.SetWeight(wxFONTWEIGHT_BOLD);
				font.SetStyle(wxFONTSTYLE_ITALIC);
			}
			if (bookmark.style != 0)
				m_tree->SetItemFont(addItemId, font);
			m_tree->SelectItem(addItemId);
		}
		dlg->Destroy();
		delete dlg;
	}
	else if (event.GetEventObject() == m_btnEdit)
	{
		wxTreeItemId itemId = m_tree->GetSelection();
		if (!itemId.IsOk())
			return;
		MyTreeItemData* itemData = (MyTreeItemData*)m_tree->GetItemData(itemId);
		int index = itemData->GetIndex();
		MyBookmark editBookmark = itemData->GetBookmark();
		bookmarkState statePrev = editBookmark.state;

		editBookmark.state = Edit;
		AddBookmarkDialog *dlg = new AddBookmarkDialog(this, editBookmark);
		int returnCode = dlg->ShowModal();
		if (returnCode == wxID_OK)
		{
			MyBookmark bookmark = dlg->GetBookmark();
			bookmark.state = statePrev;
			bookmark.delIndexes = editBookmark.delIndexes;
			m_tree->SetItemText(itemId, bookmark.name);
			m_tree->SetItemTextColour(itemId, wxColor(bookmark.color[0], bookmark.color[1], bookmark.color[2]));
			wxFont font = m_tree->GetItemFont(itemId);
			if (bookmark.style == 1) {
				font.SetStyle(wxFONTSTYLE_ITALIC);
				font.SetWeight(wxFONTWEIGHT_NORMAL);
			}
			else if (bookmark.style == 2) {
				font.SetWeight(wxFONTWEIGHT_BOLD);
				font.SetStyle(wxFONTSTYLE_NORMAL);
			}
			else if (bookmark.style == 3) {
				font.SetWeight(wxFONTWEIGHT_BOLD);
				font.SetStyle(wxFONTSTYLE_ITALIC);
			}
			if (bookmark.style != 0)
				m_tree->SetItemFont(itemId, font);
			if (index != -1) {
				m_bookmarks[index] = bookmark;
			}
			else {
				bookmark.state = Edit;
				bookmark.delIndexes = ItemIdToIndexArray(itemId);
				index = m_bookmarks.size();
				m_bookmarks.push_back(bookmark);
			}
			m_tree->SetItemData(itemId, new MyTreeItemData(index, bookmark));
		}
		dlg->Destroy();
		delete dlg;
	}
	else if (event.GetEventObject() == m_btnRemove)
	{
		wxTreeItemId itemId = m_tree->GetSelection();
		if (!itemId.IsOk())
			return;
		MyTreeItemData* itemData = (MyTreeItemData*)m_tree->GetItemData(itemId);
		int index = itemData->GetIndex();
		MyBookmark itemBookmark = itemData->GetBookmark();
		if (index != -1 && itemBookmark.state != Edit)
			m_bookmarks[index].state = Nothing;
		else {
			MyBookmark bookmark;
			bookmark.state = Delete;
			bookmark.delIndexes = ItemIdToIndexArray(itemId);
			m_bookmarks.push_back(bookmark);
		}
		m_tree->Delete(itemId);
	}
	else if (event.GetEventObject() == m_btnImport)
	{
		ImportBookmarkDialog *dlg = new ImportBookmarkDialog(this);
		int returnCode = dlg->ShowModal();
		if (returnCode == wxID_OK)
		{
		}
		dlg->Destroy();
		delete dlg;
	}
	else
	{
		event.Skip();
	}
}

void BookmarkManagerDialog::VisitAllNodes(MNode* node, wxTreeItemId parentId)
{
	wxTreeItemId itemId = m_rootId;
	if (node->Parent != nullptr)
	{
		MyBookmark bookmark = node->bookmark;
		itemId = m_tree->AppendItem(parentId, node->title, -1, -1, new MyTreeItemData(-1, bookmark));
		m_tree->SetItemTextColour(itemId, wxColor(bookmark.color[0], bookmark.color[1], bookmark.color[2]));
		wxFont font = m_tree->GetItemFont(itemId);
		if (bookmark.style == 1)
			font.SetStyle(wxFONTSTYLE_ITALIC);
		else if (bookmark.style == 2)
			font.SetWeight(wxFONTWEIGHT_BOLD);
		else if (bookmark.style == 3) {
			font.SetWeight(wxFONTWEIGHT_BOLD);
			font.SetStyle(wxFONTSTYLE_ITALIC);
		}
		if (bookmark.style != 0)
			m_tree->SetItemFont(itemId, font);
	}

	vector<MNode *> children = node->children;
	for (int i = 0; i < children.size(); ++i) {
		VisitAllNodes(children[i], itemId);
	}
}

void BookmarkManagerDialog::SetOriginBookmarks(MTree* tree)
{
	MNode* root = tree->getRoot();
	VisitAllNodes(root, m_rootId);
}

void BookmarkManagerDialog::SetOriginNamedDests(wxArrayString names)
{
	m_nameDests = names;
}

std::vector<MyBookmark> BookmarkManagerDialog::GetBookmarks()
{
	return m_bookmarks;
}

BEGIN_EVENT_TABLE(BookmarkManagerDialog, wxDialog)
EVT_BUTTON(wxID_ANY, BookmarkManagerDialog::OnButtonANY)
END_EVENT_TABLE()


// AddBookmarkDialog
AddBookmarkDialog::AddBookmarkDialog(wxWindow *parent, MyBookmark bookmark)
	: wxDialog(parent, wxID_ANY, "")
{
	wxStaticBoxSizer* sizerStaticAddBookmark = new wxStaticBoxSizer(wxVERTICAL, this, "新增书签");

	wxStaticText* statxtName = new wxStaticText(this, wxID_ANY, "名称:");
	m_textctrl = new wxTextCtrl(this, wxID_ANY);
	wxBoxSizer* sizerName = new wxBoxSizer(wxVERTICAL);
	sizerName->Add(statxtName, 0, wxALIGN_LEFT | wxALL, 5);
	sizerName->Add(m_textctrl, 0, wxEXPAND | wxALL, 5);

	wxStaticText* statxtColor = new wxStaticText(this, wxID_ANY, "颜色:");
	m_colorPicker = new wxColourPickerCtrl(this, wxID_ANY);

	wxBoxSizer* sizerColor = new wxBoxSizer(wxVERTICAL);
	sizerColor->Add(statxtColor, 0, wxALIGN_LEFT | wxALL, 5);
	sizerColor->Add(m_colorPicker, 0, wxALIGN_CENTER | wxALL, 5);

	wxStaticText* statxtStyle = new wxStaticText(this, wxID_ANY, "样式:");
	wxArrayString stringsStyle;
	stringsStyle.Add(wxT("规则"));
	stringsStyle.Add(wxT("斜体"));
	stringsStyle.Add(wxT("粗体"));
	stringsStyle.Add(wxT("粗斜体"));
	m_choiceStyle = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringsStyle);
	m_choiceStyle->SetSelection(0);

	wxBoxSizer* sizerStyle = new wxBoxSizer(wxVERTICAL);
	sizerStyle->Add(statxtStyle, 0, wxALIGN_LEFT | wxALL, 5);
	sizerStyle->Add(m_choiceStyle, 0, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer* sizerColorSytle = new wxBoxSizer(wxHORIZONTAL);
	sizerColorSytle->Add(sizerColor, 0, wxALIGN_CENTER | wxALL, 5);
	sizerColorSytle->Add(sizerStyle, 0, wxALIGN_CENTER | wxALL, 5);

	wxStaticText* statxtAction = new wxStaticText(this, wxID_ANY, "动作:");
	m_btnEditAction = new wxButton(this, wxID_ANY, "修改动作");
	wxArrayString stringsAction;
	stringsAction.Add(wxT("无"));
	stringsAction.Add(wxT("打开页面"));
	stringsAction.Add(wxT("打开文档"));
	stringsAction.Add(wxT("打开文档页面"));
	m_choiceAction = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringsAction);
	m_choiceAction->SetSelection(0);
	m_action.actionName = 0;

	wxBoxSizer* sizerActionHorizon = new wxBoxSizer(wxHORIZONTAL);
	sizerActionHorizon->Add(m_choiceAction, 0, wxALIGN_CENTER | wxALL, 5);
	sizerActionHorizon->Add(m_btnEditAction, 0, wxALIGN_CENTER | wxALL, 5);
	wxBoxSizer* sizerAction = new wxBoxSizer(wxVERTICAL);
	sizerAction->Add(statxtAction, 0, wxALIGN_LEFT | wxALL, 5);
	sizerAction->Add(sizerActionHorizon, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer* sizerAddBookmark = new wxBoxSizer(wxVERTICAL);
	sizerAddBookmark->Add(sizerName, 0, wxEXPAND | wxALL, 5);
	sizerAddBookmark->Add(sizerColorSytle, 0, wxEXPAND | wxALL, 5);
	sizerAddBookmark->Add(sizerAction, 0, wxEXPAND | wxALL, 5);

	sizerStaticAddBookmark->Add(sizerAddBookmark, wxALIGN_CENTER | wxALL, 10);

	m_btnOk = new wxButton(this, wxID_OK, "确定");
	wxButton* btnCancel = new wxButton(this, wxID_CANCEL, "取消");
	wxBoxSizer* sizerRight = new wxBoxSizer(wxVERTICAL);
	sizerRight->Add(m_btnOk, 0, wxALIGN_CENTER | wxALL, 5);
	sizerRight->Add(btnCancel, 0, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer* sizerTop = new wxBoxSizer(wxHORIZONTAL);
	sizerTop->Add(sizerStaticAddBookmark, 0, wxALIGN_CENTER | wxALL, 5);
	sizerTop->Add(sizerRight, 0, wxALIGN_CENTER | wxALL, 5);

	m_colorPicker->Enable(false);
	m_choiceStyle->Enable(false);
	m_choiceAction->Enable(false);
	m_btnOk->Enable(false);

	SetSizer(sizerTop);

	sizerTop->SetSizeHints(this);
	sizerTop->Fit(this);

	m_btnOk->SetFocus();
	m_btnOk->SetDefault();

	if (bookmark.state == Edit) {
		m_textctrl->SetLabel(bookmark.name);
		m_colorPicker->SetColour(wxColor(bookmark.color[0], bookmark.color[1], bookmark.color[2]));
		m_choiceStyle->SetSelection(bookmark.style);
		m_choiceAction->SetSelection(bookmark.action.actionName);
		m_action = bookmark.action;
	}
}

std::map<int, wxString> mapIntToActionType = {
	{ 0, "无" },
	{ 1, "打开页面" },
	{ 2, "打开文档" },
	{ 3, "打开文档页面" },
};

void AddBookmarkDialog::OnButtonANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_btnEditAction)
	{
		int actionType = m_choiceAction->GetSelection();
		switch (actionType)
		{
		case 0:
			m_action.actionName = 0;
			break;
		case 1:
		case 3:
		{
			OpenAFileViewDialog *dlg = new OpenAFileViewDialog(this, mapIntToActionType[actionType], m_action);
			int returnCode = dlg->ShowModal();
			if (returnCode == wxID_OK)
			{
				m_action = dlg->GetAction();
			}
			dlg->Destroy();
			delete dlg;
			break;
		}
		case 2:
		{
			wxFileDialog dialog(this, "Choose a File", "", "", "PDF files (*.pdf)|*.pdf", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (dialog.ShowModal() == wxID_OK)
			{
				OpenFileOptionDialog *dlg = new OpenFileOptionDialog(this);
				if (dlg->ShowModal() == wxID_OK) {
					m_action.actionName = 2;
					m_action.filepath = dialog.GetPath();
					m_action.option = dlg->GetOption();
				}
				dlg->Destroy();
				delete dlg;
			}
			break;
		}
		default:
			break;
		}
		m_choiceAction->SetSelection(m_action.actionName);
	}
	else
	{
		event.Skip();
	}
}

void AddBookmarkDialog::OnButtonOK(wxCommandEvent& event)
{
	if (m_choiceAction->GetStringSelection() == "打开文档" && m_action.filepath == "") {
		wxMessageBox("请指定相关动作", "Warning", wxOK | wxICON_WARNING, this);
	}
	else {
		m_bookmark.name = m_textctrl->GetLineText(0);
		wxColor color = m_colorPicker->GetColour();
		m_bookmark.color[0] = color.Red();
		m_bookmark.color[1] = color.Green();
		m_bookmark.color[2] = color.Blue();
		m_bookmark.style = m_choiceStyle->GetSelection();
		m_action.actionName = m_choiceAction->GetSelection();
		m_bookmark.action = m_action;
		EndModal(wxID_OK);
	}
}

std::map<wxString, int> mapActionType = {
	{ "无", 0 },
	{ "打开页面", 1 },
	{ "打开文档", 2 },
	{ "打开文档页面", 3 },
};

// 0 None
// 1 Open a view    打开页面
// 2 Open a file    打开文档
// 3 打开文档页面
void AddBookmarkDialog::OnChoiceANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_choiceAction)
	{
		wxCommandEvent e;
		e.SetEventObject(m_btnEditAction);
		OnButtonANY(e);
	}
	else
	{
		event.Skip();
	}
}

void AddBookmarkDialog::OnTextANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_textctrl)
	{
		wxCommandEvent e;
		wxString bmName = m_textctrl->GetLineText(0);
		if (bmName != "") {
			m_colorPicker->Enable(true);
			m_choiceStyle->Enable(true);
			m_choiceAction->Enable(true);
			m_btnOk->Enable(true);
		}
		else {
			m_colorPicker->Enable(false);
			m_choiceStyle->Enable(false);
			m_choiceAction->Enable(false);
			m_btnOk->Enable(false);
		}
	}
	else
	{
		event.Skip();
	}
}

MyBookmark AddBookmarkDialog::GetBookmark()
{
	return m_bookmark;
}

BEGIN_EVENT_TABLE(AddBookmarkDialog, wxDialog)
EVT_BUTTON(wxID_ANY, AddBookmarkDialog::OnButtonANY)
EVT_BUTTON(wxID_OK, AddBookmarkDialog::OnButtonOK)
EVT_CHOICE(wxID_ANY, AddBookmarkDialog::OnChoiceANY)
EVT_TEXT(wxID_ANY, AddBookmarkDialog::OnTextANY)
END_EVENT_TABLE()

OpenAFileViewDialog::OpenAFileViewDialog(wxWindow *parent, wxString title, MyBookmarkAction action)
	: wxDialog(parent, wxID_ANY, title)
{
	wxStaticText* statxtFile = new wxStaticText(this, wxID_ANY, "文件:");
	m_textFile = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_btnChooseFile = new wxButton(this, wxID_ANY, "浏览...");
	wxBoxSizer* sizerFile = new wxBoxSizer(wxHORIZONTAL);
	sizerFile->Add(statxtFile, 0, wxEXPAND | wxALL, 5);
	sizerFile->Add(m_textFile, 0, wxEXPAND | wxALL, 5);
	sizerFile->Add(m_btnChooseFile, 0, wxEXPAND | wxALL, 5);

	wxStaticText* statxtOption = new wxStaticText(this, wxID_ANY, "打开于:");
	wxArrayString stringsOption;
	stringsOption.Add(wxT("用户首选项设置的窗口"));
	stringsOption.Add(wxT("新建窗口"));
	stringsOption.Add(wxT("现有窗口"));
	m_choiceOption = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringsOption);
	m_choiceOption->SetSelection(0);
	wxBoxSizer* sizerOption = new wxBoxSizer(wxHORIZONTAL);
	sizerOption->Add(statxtOption, 0, wxALIGN_CENTER | wxALL, 5);
	sizerOption->Add(m_choiceOption, 0, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer *sizerFileAndOption = new wxBoxSizer(wxVERTICAL);
	sizerFileAndOption->Add(sizerFile, 0, wxALIGN_LEFT | wxALL, 5);
	sizerFileAndOption->Add(sizerOption, 0, wxALIGN_LEFT | wxALL, 5);
	wxStaticBoxSizer *sizerGoal;
	if (title == "打开文档页面") {
		sizerGoal = new wxStaticBoxSizer(wxVERTICAL, this, "目标文档");
		sizerGoal->Add(sizerFileAndOption, 0, wxEXPAND | wxALL, 10);
	}


	wxStaticBoxSizer *sizerGoalOption = new wxStaticBoxSizer(wxVERTICAL, this, "选项");

	m_rbtnPage = new wxRadioButton(this, wxID_ANY, "使用页码", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_rbtnPage->SetValue(true);
	wxStaticText* statxtPage = new wxStaticText(this, wxID_ANY, "页码:");
	m_textPage = new wxTextCtrl(this, wxID_ANY);
	m_textPage->SetValue("1");
	wxStaticText* statxtChar = new wxStaticText(this, wxID_ANY, "/");
	m_textPageNum = new wxStaticText(this, wxID_ANY, "1");
	wxBoxSizer* sizerPage = new wxBoxSizer(wxHORIZONTAL);
	sizerPage->Add(statxtPage, 0, wxALIGN_CENTER | wxALL, 5);
	sizerPage->Add(m_textPage, 0, wxALIGN_CENTER | wxALL, 5);
	sizerPage->Add(statxtChar, 0, wxALIGN_CENTER | wxALL, 5);
	sizerPage->Add(m_textPageNum, 0, wxALIGN_CENTER | wxALL, 5);

	wxStaticText* statxtMagni = new wxStaticText(this, wxID_ANY, "放大率:");
	wxArrayString stringsMagni;
	stringsMagni.Add("适合页面");
	stringsMagni.Add("实际大小");
	stringsMagni.Add("适合宽度");
	stringsMagni.Add("适合可见");
	stringsMagni.Add("承前缩放");
	stringsMagni.Add("缩放");
	m_choiceMagni = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringsMagni);
	m_choiceMagni->SetSelection(0);

	m_textMagni = new wxTextCtrl(this, wxID_ANY);
	m_textMagni->SetLabel("100.0");
	m_textMagni->Enable(false);
	wxStaticText* statxtPercent = new wxStaticText(this, wxID_ANY, "%");
	wxBoxSizer* sizerMagni = new wxBoxSizer(wxHORIZONTAL);
	sizerMagni->Add(statxtMagni, 0, wxALIGN_CENTER | wxALL, 5);
	sizerMagni->Add(m_choiceMagni, 0, wxALIGN_CENTER | wxALL, 5);
	sizerMagni->Add(m_textMagni, 0, wxALIGN_CENTER | wxALL, 5);
	sizerMagni->Add(statxtPercent, 0, wxALIGN_CENTER | wxALL, 5);

	m_rbtnNamedDest = new wxRadioButton(this, wxID_ANY, "使用已命名的目标");
	wxStaticText* statxtNamedDest = new wxStaticText(this, wxID_ANY, "名称:");
	m_textNamedDest = new wxStaticText(this, wxID_ANY, "未指定");
	m_btnChooseNamedDest = new wxButton(this, wxID_ANY, "浏览...");
	m_btnChooseNamedDest->Enable(false);
	wxBoxSizer* sizerNamedDest = new wxBoxSizer(wxHORIZONTAL);
	sizerNamedDest->Add(statxtNamedDest, 0, wxEXPAND | wxALL, 5);
	sizerNamedDest->Add(m_textNamedDest, 0, wxEXPAND | wxALL, 5);
	sizerNamedDest->Add(m_btnChooseNamedDest, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer* sizerPageDest = new wxBoxSizer(wxVERTICAL);
	sizerPageDest->Add(m_rbtnPage, 0, wxEXPAND | wxALL, 5);
	sizerPageDest->Add(sizerPage, 0, wxEXPAND | wxALL, 15);
	sizerPageDest->Add(sizerMagni, 0, wxEXPAND | wxALL, 15);
	sizerPageDest->Add(m_rbtnNamedDest, 0, wxEXPAND | wxALL, 5);
	sizerPageDest->Add(sizerNamedDest, 0, wxEXPAND | wxALL, 15);
	sizerGoalOption->Add(sizerPageDest, 0, wxALIGN_CENTER | wxALL, 10);

	m_btnOk = new wxButton(this, wxID_OK, "确定");
	wxButton* btnCancel = new wxButton(this, wxID_CANCEL, "取消");
	wxBoxSizer* sizerOkCancel = new wxBoxSizer(wxHORIZONTAL);
	sizerOkCancel->Add(m_btnOk, 0, wxALIGN_CENTER | wxALL, 5);
	sizerOkCancel->Add(btnCancel, 0, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	if (title == "打开文档页面")
		sizerTop->Add(sizerGoal, 0, wxEXPAND | wxALL, 5);
	sizerTop->Add(sizerGoalOption, 0, wxEXPAND | wxALL, 5);
	sizerTop->Add(sizerOkCancel, 0, wxALIGN_RIGHT | wxALL, 5);

	if (action.actionName == mapActionType[title])
	{
		m_textFile->SetLabel(action.filepath);
		m_choiceOption->SetStringSelection(action.option);
		if (action.useNamedDest) {
			m_textNamedDest->SetLabel(action.destName);
			m_rbtnNamedDest->SetValue(true);
			m_btnChooseNamedDest->Enable();
		}
		else {
			m_textPage->SetLabel(wxString::Format("%i", action.page));
			m_choiceMagni->SetStringSelection(action.zoomType);
			if (action.zoomType == "缩放") {
				m_textMagni->SetLabel(wxString::Format("%.1f", action.zoomRate * 100));
				m_textMagni->Enable(true);
			}
		}
	}

	if (this->GetTitle() == "打开页面") {
		AVDoc avCurrentDoc = AVAppGetActiveDoc();
		PDDoc pdDoc = AVDocGetPDDoc(avCurrentDoc);
		m_pageNum = PDDocGetNumPages(pdDoc);
		m_textPageNum->SetLabel(wxString::Format("%i", m_pageNum));
		statxtFile->Hide();
		m_textFile->Hide();
		m_btnChooseFile->Hide();
		statxtOption->Hide();
		m_choiceOption->Hide();
	}
	else if (this->GetTitle() == "打开文档页面" && m_textFile->GetLineText(0) != "") {
		ASAtom pathType = ASAtomFromString("Cstring");
		ASFileSys fileSys = ASGetDefaultFileSysForPath(pathType, action.filepath);
		ASPathName pathName = ASFileSysCreatePathName(fileSys, pathType, action.filepath, NULL);
		PDDoc pdDoc = PDDocOpen(pathName, fileSys, NULL, false);
		m_pageNum = PDDocGetNumPages(pdDoc);
		m_textPageNum->SetLabel(wxString::Format("%i", m_pageNum));
	}

	SetSizer(sizerTop);

	sizerTop->SetSizeHints(this);
	sizerTop->Fit(this);

	m_btnOk->SetFocus();
	m_btnOk->SetDefault();
}

void OpenAFileViewDialog::OnButtonANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_btnChooseFile) {
		wxFileDialog dialog(this, "Choose a File", "", "", "PDF files (*.pdf)|*.pdf", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (dialog.ShowModal() == wxID_OK) {
			wxString path = dialog.GetPath();
			m_textFile->SetLabel(path);
			m_textNamedDest->SetLabel("未指定");

			ASAtom pathType = ASAtomFromString("Cstring");
			ASFileSys fileSys = ASGetDefaultFileSysForPath(pathType, path);
			ASPathName pathName = ASFileSysCreatePathName(fileSys, pathType, path, NULL);
			PDDoc pdDoc = PDDocOpen(pathName, fileSys, NULL, false);
			m_pageNum = PDDocGetNumPages(pdDoc);
			m_textPageNum->SetLabel(wxString::Format("%i", m_pageNum));
		}
	}
	else if (event.GetEventObject() == m_btnChooseNamedDest) {
		PDDoc pdDoc;
		if (this->GetTitle() == "打开文档页面") {
			if (m_textFile->GetLineText(0) == "") {
				wxMessageBox("请指定文件路径", "Warning", wxOK | wxICON_WARNING, this);
				return;
			}
			else {
				wxString myPath = m_textFile->GetLineText(0);
				ASAtom pathType = ASAtomFromString("Cstring");
				ASFileSys fileSys = ASGetDefaultFileSysForPath(pathType, myPath);
				ASPathName pathName = ASFileSysCreatePathName(fileSys, pathType, myPath, NULL);
				pdDoc = PDDocOpen(pathName, fileSys, NULL, false);
			}
		}
		else {
			AVDoc avCurrentDoc = AVAppGetActiveDoc();
			pdDoc = AVDocGetPDDoc(avCurrentDoc);
		}
		wxArrayString names = GetOriginNamedDest(pdDoc);
		GetNamedDestDialog *dlg = new GetNamedDestDialog(this, names);
		if (dlg->ShowModal() == wxID_OK) {
			wxString name = dlg->GetName();
			if (name != "")
				m_textNamedDest->SetLabel(name);
		}
		dlg->Destroy();
		delete dlg;
	}
	else {
		event.Skip();
	}
}

void OpenAFileViewDialog::OnButtonOK(wxCommandEvent& event)
{
	wxString title = this->GetTitle();
	if (title == "打开文档页面" && m_textFile->GetLineText(0) == "") {
		wxMessageBox("请指定文件路径", "Warning", wxOK | wxICON_WARNING, this);
		return;
	}
	else {
		m_action.actionName = mapActionType[title];
		m_action.filepath = m_textFile->GetLineText(0);
		m_action.option = m_choiceOption->GetStringSelection();

		if (m_rbtnPage->GetValue()) {
			int page;
			float zoomRate;
			regex r("[0-9]+");
			string stringPage = m_textPage->GetLineText(0);
			if (! regex_match(stringPage, r)) {
				wxMessageBox("请输入正确的页码", "Warning", wxOK | wxICON_WARNING, this);
				return;
			}
			page = wxAtoi(stringPage);
			if (page > m_pageNum || page < 1) {
				wxMessageBox("请输入正确的页码", "Warning", wxOK | wxICON_WARNING, this);
				return;
			}

			wxString zoomType = m_choiceMagni->GetStringSelection();
			if (zoomType == "缩放") {
				regex r_int("[0-9]+");
				regex r_float("[0-9]+[.][0-9]+");
				string stringZoom = m_textMagni->GetLineText(0);
				if (!regex_match(stringZoom, r_int) && !regex_match(stringZoom, r_float)) {
					wxMessageBox("请输入正确的放大率", "Warning", wxOK | wxICON_WARNING, this);
					return;
				}
				zoomRate = wxAtof(stringZoom) / 100;
				if (zoomRate <= 0) {
					wxMessageBox("请输入正确的放大率", "Warning", wxOK | wxICON_WARNING, this);
					return;
				}
			}
			m_action.useNamedDest = false;
			m_action.page = page;
			m_action.zoomType = zoomType;
			m_action.zoomRate = zoomRate;
		}
		else {
			m_action.useNamedDest = true;
			wxString goal = m_textNamedDest->GetLabel();
			if (goal == "未指定") {
				wxMessageBox("请指定目标", "Warning", wxOK | wxICON_WARNING, this);
				return;
			}
			else
				m_action.destName = goal;
		}
	}
	EndModal(wxID_OK);
}

void OpenAFileViewDialog::OnChoiceANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_choiceMagni)
	{
		if (m_choiceMagni->GetStringSelection() == "缩放")
			m_textMagni->Enable(true);
		else
			m_textMagni->Enable(false);
	}
	else
	{
		event.Skip();
	}
}

void OpenAFileViewDialog::OnRadiobuttonANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_rbtnPage) {
		m_btnChooseNamedDest->Enable(false);
		m_textPage->Enable(true);
		m_choiceMagni->Enable(true);
		if (m_choiceMagni->GetStringSelection() == "缩放")
			m_textMagni->Enable(true);
	}
	else if (event.GetEventObject() == m_rbtnNamedDest) {
		m_btnChooseNamedDest->Enable(true);
		m_textPage->Enable(false);
		m_choiceMagni->Enable(false);
		m_textMagni->Enable(false);
	}
	else
	{
		event.Skip();
	}
}

MyBookmarkAction OpenAFileViewDialog::GetAction()
{
	return m_action;
}

BEGIN_EVENT_TABLE(OpenAFileViewDialog, wxDialog)
EVT_BUTTON(wxID_ANY, OpenAFileViewDialog::OnButtonANY)
EVT_BUTTON(wxID_OK, OpenAFileViewDialog::OnButtonOK)
EVT_CHOICE(wxID_ANY, OpenAFileViewDialog::OnChoiceANY)
EVT_RADIOBUTTON(wxID_ANY, OpenAFileViewDialog::OnRadiobuttonANY)
END_EVENT_TABLE()

OpenFileOptionDialog::OpenFileOptionDialog(wxWindow *parent)
	: wxDialog(parent, wxID_ANY, "")
{
	wxArrayString stringsOption;
	stringsOption.Add(wxT("用户首选项设置的窗口"));
	stringsOption.Add(wxT("新建窗口"));
	stringsOption.Add(wxT("现有窗口"));
	m_radioBox = new wxRadioBox(this, wxID_ANY, "请指定打开首选项", wxDefaultPosition, wxDefaultSize, stringsOption, 1, wxRA_SPECIFY_COLS);

	wxButton *btnOk = new wxButton(this, wxID_OK, "确定");

	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	sizerTop->Add(m_radioBox, 0, wxALIGN_CENTER | wxALL, 10);
	sizerTop->Add(btnOk, 0, wxALIGN_CENTER | wxALL, 10);

	SetSizer(sizerTop);

	sizerTop->SetSizeHints(this);
	sizerTop->Fit(this);

	btnOk->SetFocus();
	btnOk->SetDefault();
}

void OpenFileOptionDialog::OnButtonOK(wxCommandEvent& event)
{
	m_option = m_radioBox->GetStringSelection();
	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(OpenFileOptionDialog, wxDialog)
EVT_BUTTON(wxID_OK, OpenFileOptionDialog::OnButtonOK)
END_EVENT_TABLE()

GetNamedDestDialog::GetNamedDestDialog(wxWindow *parent, wxArrayString names)
	: wxDialog(parent, wxID_ANY, "")
{
	m_list = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(200, 240));
	if (!names.IsEmpty())
		m_list->InsertItems(names, 0);

	wxButton *btnOk = new wxButton(this, wxID_OK, "确定");
	wxButton *btnCancel = new wxButton(this, wxID_CANCEL, "取消");
	wxBoxSizer* sizerBtn = new wxBoxSizer(wxHORIZONTAL);
	sizerBtn->Add(btnOk, 0, wxALIGN_CENTER | wxALL, 5);
	sizerBtn->Add(btnCancel, 0, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	sizerTop->Add(m_list, 0, wxALIGN_CENTER | wxALL, 10);
	sizerTop->Add(sizerBtn, 0, wxALIGN_CENTER | wxALL, 10);

	SetSizer(sizerTop);

	sizerTop->SetSizeHints(this);
	sizerTop->Fit(this);

	btnOk->SetFocus();
	btnOk->SetDefault();
}

void GetNamedDestDialog::OnButtonOK(wxCommandEvent& event)
{
	m_name = m_list->GetStringSelection();
	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(GetNamedDestDialog, wxDialog)
EVT_BUTTON(wxID_OK, GetNamedDestDialog::OnButtonOK)
END_EVENT_TABLE()

ImportBookmarkDialog::ImportBookmarkDialog(wxWindow *parent)
	: wxDialog(parent, wxID_ANY, "将TOC转换为书签")
{
	wxStaticBoxSizer *sizerPos = new wxStaticBoxSizer(wxVERTICAL, this, "TOC位置");
	m_rbtnPage = new wxRadioButton(this, wxID_ANY, "扫描", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_rbtnRange = new wxRadioButton(this, wxID_ANY, "范围");
	m_textRange = new wxTextCtrl(this, wxID_ANY);
	m_textRange->Enable(false);
	wxArrayString stringChoicePage;
	stringChoicePage.Add("第一页");
	stringChoicePage.Add("第二页");
	stringChoicePage.Add("最后一页");
	stringChoicePage.Add("当前页");
	m_choicePage = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringChoicePage);
	m_choicePage->SetSelection(0);
	wxBoxSizer *sizerPage = new wxBoxSizer(wxHORIZONTAL);
	sizerPage->Add(m_rbtnPage, 0, wxEXPAND | wxALL, 5);
	sizerPage->Add(m_choicePage, 0, wxEXPAND | wxALL, 5);
	wxBoxSizer *sizerRange = new wxBoxSizer(wxHORIZONTAL);
	sizerRange->Add(m_rbtnRange, 0, wxEXPAND | wxALL, 5);
	sizerRange->Add(m_textRange, 0, wxEXPAND | wxALL, 5);
	sizerPos->Add(sizerPage, wxEXPAND | wxALL, 10);
	sizerPos->Add(sizerRange, wxEXPAND | wxALL, 10);

	wxStaticBoxSizer *sizerTitle = new wxStaticBoxSizer(wxVERTICAL, this, "书签标题");
	m_ckboxSeperator = new wxCheckBox(this, wxID_ANY, "删除分隔符");
	m_ckboxNumber = new wxCheckBox(this, wxID_ANY, "删除标题末尾数字");
	wxArrayString stringChoiceSeperator;
	stringChoiceSeperator.Add("点......");
	stringChoiceSeperator.Add("下划线______");
	stringChoiceSeperator.Add("连字符------");
	stringChoiceSeperator.Add("用户自定义");
	m_choiceSeperator = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringChoiceSeperator);
	m_choiceSeperator->SetSelection(0);
	m_choiceSeperator->Enable(false);
	m_textSeperator = new wxTextCtrl(this, wxID_ANY);
	m_textSeperator->Enable(false);
	wxBoxSizer *sizerRight = new wxBoxSizer(wxVERTICAL);
	sizerRight->Add(m_choiceSeperator, 0, wxEXPAND | wxALL, 5);
	sizerRight->Add(m_textSeperator, 0, wxEXPAND | wxALL, 5);
	wxBoxSizer *sizerSeperator = new wxBoxSizer(wxHORIZONTAL);
	sizerSeperator->Add(m_ckboxSeperator, 0, wxALIGN_TOP | wxALL, 15);
	sizerSeperator->Add(sizerRight, 0, wxALIGN_TOP | wxALL, 5);
	sizerTitle->Add(sizerSeperator, wxALIGN_LEFT | wxALL, 2);
	sizerTitle->Add(m_ckboxNumber, wxALIGN_LEFT | wxALL, 2);

	wxButton *btnOk = new wxButton(this, wxID_OK, "确定");
	wxButton *btnCancel = new wxButton(this, wxID_CANCEL, "取消");
	wxBoxSizer* sizerBtn = new wxBoxSizer(wxHORIZONTAL);
	sizerBtn->Add(btnOk, 0, wxALIGN_CENTER | wxALL, 5);
	sizerBtn->Add(btnCancel, 0, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	sizerTop->Add(sizerPos, 0, wxEXPAND | wxALL, 10);
	sizerTop->Add(sizerTitle, 0, wxALIGN_CENTER | wxALL, 10);
	sizerTop->Add(sizerBtn, 0, wxALIGN_RIGHT | wxALL, 5);

	SetSizer(sizerTop);

	sizerTop->SetSizeHints(this);
	sizerTop->Fit(this);

	btnOk->SetFocus();
	btnOk->SetDefault();
}

void ImportBookmarkDialog::OnButtonOK(wxCommandEvent& event)
{
	if (m_rbtnRange->GetValue()) {
		AVDoc avCurrentDoc = AVAppGetActiveDoc();
		PDDoc pdDoc = AVDocGetPDDoc(avCurrentDoc);
		//GetTOC(pdDoc, wxAtoi(m_textRange->GetLineText(0)));
	}
	EndModal(wxID_OK);
}

void ImportBookmarkDialog::OnChoiceANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_choiceSeperator)
	{
		if (m_choiceSeperator->GetStringSelection() == "用户自定义")
			m_textSeperator->Enable(true);
		else
			m_textSeperator->Enable(false);
	}
	else
	{
		event.Skip();
	}
}

void ImportBookmarkDialog::OnRadiobuttonANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_rbtnPage) {
		m_choicePage->Enable(true);
		m_textRange->Enable(false);
	}
	else if (event.GetEventObject() == m_rbtnRange) {
		m_choicePage->Enable(false);
		m_textRange->Enable(true);
	}
	else
	{
		event.Skip();
	}
}

void  ImportBookmarkDialog::OnCheckBoxANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_ckboxSeperator) {
		if(m_ckboxSeperator->IsChecked()){
			m_choiceSeperator->Enable(true);
			wxCommandEvent e;
			e.SetEventObject(m_choiceSeperator);
			OnChoiceANY(e);
		}
		else {
			m_choiceSeperator->Enable(false);
			m_textSeperator->Enable(false);
		}
	}
}

BEGIN_EVENT_TABLE(ImportBookmarkDialog, wxDialog)
EVT_RADIOBUTTON(wxID_ANY, ImportBookmarkDialog::OnRadiobuttonANY)
EVT_BUTTON(wxID_OK, ImportBookmarkDialog::OnButtonOK)
EVT_CHECKBOX(wxID_ANY, ImportBookmarkDialog::OnCheckBoxANY)
EVT_CHOICE(wxID_ANY, ImportBookmarkDialog::OnChoiceANY)
END_EVENT_TABLE()