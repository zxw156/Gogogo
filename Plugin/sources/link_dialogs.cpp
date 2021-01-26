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

 link_dialogs.cpp

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

#include "link_dialogs.h"
#include <regex>

#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#endif

ACCB1 wxArrayString ACCB2 GetOriginNamedDest(PDDoc doc);
ACCB1 MTree* ACCB2 GetOriginBookmarks(PDDoc doc);

MTree* getCurrentBookmark()
{
	AVDoc avCurrentDoc = AVAppGetActiveDoc();
	PDDoc pdDoc = AVDocGetPDDoc(avCurrentDoc);
	return GetOriginBookmarks(pdDoc);
}

MTree* getFileBookmark(wxString filename)
{
	ASAtom pathType = ASAtomFromString("Cstring");
	ASFileSys fileSys = ASGetDefaultFileSysForPath(pathType, filename);
	ASPathName pathName = ASFileSysCreatePathName(fileSys, pathType, filename, NULL);
	PDDoc pdDoc = PDDocOpen(pathName, fileSys, NULL, false);
	return GetOriginBookmarks(pdDoc);
}

wxArrayString getCurrentNamedDest()
{
	AVDoc avCurrentDoc = AVAppGetActiveDoc();
	PDDoc pdDoc = AVDocGetPDDoc(avCurrentDoc);
	return GetOriginNamedDest(pdDoc);
}

wxArrayString getFileNamedDest(wxString filename)
{
	//AVAlertNote(filename);
	ASAtom pathType = ASAtomFromString("Cstring");
	ASFileSys fileSys = ASGetDefaultFileSysForPath(pathType, filename);
	ASPathName pathName = ASFileSysCreatePathName(fileSys, pathType, filename, NULL);
	PDDoc pdDoc = PDDocOpen(pathName, fileSys, NULL, false);
	return GetOriginNamedDest(pdDoc);
}

int  getCurrentPageNum()
{
	AVDoc avCurrentDoc = AVAppGetActiveDoc();
	PDDoc pdDoc = AVDocGetPDDoc(avCurrentDoc);
	return PDDocGetNumPages(pdDoc);
}

int  getFilePageNum(wxString filename)
{
	ASAtom pathType = ASAtomFromString("Cstring");
	ASFileSys fileSys = ASGetDefaultFileSysForPath(pathType, filename);
	ASPathName pathName = ASFileSysCreatePathName(fileSys, pathType, filename, NULL);
	PDDoc pdDoc = PDDocOpen(pathName, fileSys, NULL, false);
	return  PDDocGetNumPages(pdDoc);
}

// ----------------------------------------------------------------------------
// PanelDest
// ----------------------------------------------------------------------------
PanelDest::PanelDest(wxWindow *parent)
	: wxScrolled(parent, wxID_ANY, wxDefaultPosition, wxSize(300, 350), wxVSCROLL | wxRESIZE_BORDER)
{
	this->SetScrollbars(20, 20, 50, 50);

	wxStaticBoxSizer *sizerBehavior = new wxStaticBoxSizer(wxVERTICAL, this, "行为");

	wxBoxSizer *sizerLinkType = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *textLinkType = new wxStaticText(this, wxID_ANY, "链接类型:"); 
	wxArrayString stringLinkType;
	stringLinkType.Add("打开页面");
	stringLinkType.Add("打开文件");
	stringLinkType.Add("打开文件页面");
	m_choiceLinkType = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringLinkType);
	m_choiceLinkType->SetSelection(0);
	sizerLinkType->Add(textLinkType, 0, wxALIGN_CENTER | wxALL, 5);
	sizerLinkType->Add(m_choiceLinkType, 1, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer *sizerFilePath = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *staticTextFilePath = new wxStaticText(this, wxID_ANY, "文件路径:");
	m_textFilePath = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_btnFile = new wxButton(this, wxID_ANY, "...");
	m_btnFile->Enable(false);
	sizerFilePath->Add(staticTextFilePath, 0, wxALIGN_CENTER | wxALL, 5);
	sizerFilePath->Add(m_textFilePath, 1, wxALIGN_CENTER | wxALL, 5);
	sizerFilePath->Add(m_btnFile, 1, wxALIGN_CENTER | wxALL, 5);

	wxStaticBoxSizer *sizerLinkTarget = new wxStaticBoxSizer(wxVERTICAL, this, "链接目标");
	m_rbtnFirstPage = new wxRadioButton(this, wxID_ANY, "首页", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_rbtnFirstPage->Enable(false);
	wxBoxSizer* sizerFirstPage = new wxBoxSizer(wxHORIZONTAL);
	sizerFirstPage->Add(m_rbtnFirstPage, 0, wxALIGN_CENTER | wxALL, 5);

	m_rbtnPage = new wxRadioButton(this, wxID_ANY, "指定页面");
	m_rbtnPage->SetValue(true);
	m_spinPage = new wxSpinCtrl(this, wxID_ANY, "1");
	m_spinPage->SetMin(1);
	wxStaticText* statxtChar = new wxStaticText(this, wxID_ANY, "/");
	m_currentPageNum = getCurrentPageNum();
	m_spinPage->SetMax(m_currentPageNum);
	m_textPageNum = new wxStaticText(this, wxID_ANY, wxString::Format("%i", m_currentPageNum));
	wxBoxSizer* sizerPage = new wxBoxSizer(wxHORIZONTAL);
	sizerPage->Add(m_rbtnPage, 1, wxALIGN_CENTER | wxALL, 5);
	sizerPage->Add(m_spinPage, 1, wxALIGN_CENTER | wxALL, 5);
	sizerPage->Add(statxtChar, 1, wxALIGN_CENTER | wxALL, 5);
	sizerPage->Add(m_textPageNum, 1, wxALIGN_CENTER | wxALL, 5);

	m_rbtnNamedDest = new wxRadioButton(this, wxID_ANY, "指定文件目标");
	m_currentNamedDest = getCurrentNamedDest();
	m_choiceNamedDest = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(80, 20), m_currentNamedDest);
	m_choiceNamedDest->SetSelection(0);
	wxBoxSizer* sizerNamedDest = new wxBoxSizer(wxHORIZONTAL);
	sizerNamedDest->Add(m_rbtnNamedDest, 1, wxALIGN_CENTER | wxALL, 5);
	sizerNamedDest->Add(m_choiceNamedDest, 1, wxALIGN_CENTER | wxALL, 5);

	m_rbtnBookmark = new wxRadioButton(this, wxID_ANY, "书签");
	m_btnBookmark = new wxButton(this, wxID_ANY, "选择书签");
	m_textBookmark = new wxStaticText(this, wxID_ANY, "");
	m_currentBookmark = getCurrentBookmark();
	wxBoxSizer* sizerBookmark = new wxBoxSizer(wxHORIZONTAL);
	sizerBookmark->Add(m_rbtnBookmark, 1, wxALIGN_CENTER | wxALL, 5);
	sizerBookmark->Add(m_btnBookmark, 1, wxALIGN_CENTER | wxALL, 5);
	sizerBookmark->Add(m_textBookmark, 1, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer* sizerLinkGoal = new wxBoxSizer(wxVERTICAL);
	sizerLinkGoal->Add(sizerFirstPage, 1, wxALIGN_LEFT | wxALL, 5);
	sizerLinkGoal->Add(sizerPage, 1, wxALIGN_LEFT | wxALL, 5);
	sizerLinkGoal->Add(sizerNamedDest, 1, wxALIGN_LEFT | wxALL, 5);
	sizerLinkGoal->Add(sizerBookmark, 1, wxALIGN_LEFT | wxALL, 5);
	sizerLinkTarget->Add(sizerLinkGoal, 1, wxALIGN_LEFT | wxALL, 10);

	sizerBehavior->Add(sizerLinkType, 1, wxEXPAND | wxALL, 10);
	sizerBehavior->Add(sizerFilePath, 1, wxEXPAND | wxALL, 10);
	sizerBehavior->Add(sizerLinkTarget, 5, wxEXPAND | wxALL, 10);

	wxBoxSizer *sizerTop = new wxBoxSizer(wxVERTICAL);
	sizerTop->Add(sizerBehavior, 0, wxALIGN_CENTER | wxALL, 5);

	SetSizer(sizerTop);
	SetAutoLayout(true);
	//Layout();

	sizerTop->SetSizeHints(this);
	sizerTop->Fit(this);
}

void PanelDest::OnButtonANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_btnFile) {
		wxFileDialog dialog(this, "Choose a File", "", "", "PDF files (*.pdf)|*.pdf", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (dialog.ShowModal() == wxID_OK) {
			wxString path = dialog.GetPath();
			m_textFilePath->SetLabel(path);

			int pageNum = getFilePageNum(path);
			m_spinPage->SetMax(pageNum);
			m_spinPage->SetValue(1);
			m_textPageNum->SetLabel(wxString::Format("%i", pageNum));

			if (m_choiceLinkType->GetStringSelection() == "打开文件页面") {
				wxArrayString namedDests = getFileNamedDest(path);
				m_choiceNamedDest->Set(namedDests);
				m_choiceNamedDest->SetSelection(0);
				m_fileBookmark = getFileBookmark(path);
			}
		}
	}
	else if (event.GetEventObject() == m_btnBookmark){
		PDDoc pdDoc;
		MTree* bookmarks;
		if (m_choiceLinkType->GetStringSelection() == "打开文件页面") {
			wxString filename = m_textFilePath->GetLineText(0);
			if ( filename== "") {
				wxMessageBox("请指定文件路径", "Warning", wxOK | wxICON_WARNING, this);
				return;
			}
			else {
				bookmarks = m_fileBookmark;
			}
		}
		else {
			bookmarks = m_currentBookmark;
		}

		GetBookmarkDialog *dlg = new GetBookmarkDialog(this, bookmarks);
		if (dlg->ShowModal() == wxID_OK) {
			m_bookmark = dlg->GetBookmark();
			m_textBookmark->SetLabel(m_bookmark.name);
		}
		dlg->Destroy();
		delete dlg;
	}
	else
	{
		event.Skip();
	}
}

void PanelDest::OnChoiceANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_choiceLinkType)
	{
		int select = m_choiceLinkType->GetSelection();
		switch (select) {
		case 0:		// 打开页面
		case 2:		// 打开文件页面
			m_rbtnFirstPage->Enable(false);
			if (m_rbtnFirstPage->GetValue() == true)
				m_rbtnPage->SetValue(true);
			m_rbtnBookmark->Enable(true);
			m_btnBookmark->Enable(true);
			m_rbtnNamedDest->Enable(true);
			m_choiceNamedDest->Enable(true);
			m_rbtnPage->Enable(true);
			m_spinPage->Enable(true);
			break;
		case 1:
			m_rbtnFirstPage->Enable(true);
			m_rbtnFirstPage->SetValue(true);
			m_rbtnBookmark->Enable(false);
			m_btnBookmark->Enable(false);
			m_rbtnNamedDest->Enable(false);
			m_choiceNamedDest->Enable(false);
			m_rbtnPage->Enable(false);
			m_spinPage->Enable(false);
			break;
		default:
			break;
		}
		if (select == 0) {	// 打开页面
			m_textFilePath->Enable(false);
			m_textFilePath->SetLabel("");
			m_btnFile->Enable(false);
			m_spinPage->SetValue(1);
			m_textPageNum->SetLabel(wxString::Format("%i", m_currentPageNum));
			m_choiceNamedDest->Set(m_currentNamedDest);
			m_choiceNamedDest->SetSelection(0);
		}
		else {
			m_textFilePath->Enable(true);
			m_btnFile->Enable(true);
			m_choiceNamedDest->Clear();
		}
	}
	else
	{
		event.Skip();
	}
}

MyLinkDest PanelDest::getDest()
{
	MyLinkDest dest;
	MyBookmarkAction action;

	if (m_choiceLinkType->GetStringSelection() != "打开页面" && m_textFilePath->GetLineText(0) == "") {
		wxMessageBox("请指定文件路径", "Warning", wxOK | wxICON_WARNING, this);
		dest.isValid = false;
	}
	else {
		action.actionName = m_choiceLinkType->GetSelection() + 1;
		action.filepath = m_textFilePath->GetLineText(0);

		if (m_rbtnPage->GetValue()) {
			action.page = m_spinPage->GetValue();
			action.useNamedDest = false;
			dest.useBookmark = false;
		}
		else if (m_rbtnNamedDest->GetValue()) {
			action.useNamedDest = true;
			dest.useBookmark = false;
			wxString goal = m_choiceNamedDest->GetStringSelection();
			if (goal == "") {
				wxMessageBox("请指定目标", "Warning", wxOK | wxICON_WARNING, this);
				dest.isValid = false;
			}
			else
				action.destName = goal;
		}
		else if (m_rbtnBookmark->GetValue()) {
			if (m_textBookmark->GetLabel() == "") {
				wxMessageBox("请指定书签", "Warning", wxOK | wxICON_WARNING, this);
				dest.isValid = false;
			}
			else if (m_choiceLinkType->GetStringSelection() == "打开页面") {
				dest.useBookmark = false;
				action = m_bookmark.action;
				//dest.useBookmark = true;
				//dest.bookmarkAction = m_bookmark.pdAction;
			}
			else if (m_choiceLinkType->GetStringSelection() == "打开文件页面") {
				dest.useBookmark = false;
				action = m_bookmark.action;
				if (action.actionName == 1) {
					action.filepath = m_textFilePath->GetLineText(0);
					action.actionName = 3;
				}
				//action.actionName = m_choiceLinkType->GetSelection() + 1;
				//AVAlertNote(wxString::Format("%i %s", action.actionName, action.filepath));
			}
		}
	}
	dest.action = action;
	return dest;
}

BEGIN_EVENT_TABLE(PanelDest, wxScrolled)
EVT_BUTTON(wxID_ANY, PanelDest::OnButtonANY)
EVT_CHOICE(wxID_ANY, PanelDest::OnChoiceANY)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// PanelDest
// ----------------------------------------------------------------------------
PanelStyle::PanelStyle(wxWindow *parent)
	: wxScrolled(parent, wxID_ANY, wxDefaultPosition, wxSize(300, 350), wxVSCROLL | wxRESIZE_BORDER)
{
	this->SetScrollbars(20, 20, 50, 50);
	wxStaticBoxSizer *sizerLinkSetting = new wxStaticBoxSizer(wxVERTICAL, this, "链接设置");

	wxBoxSizer *sizerLinkType = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* textLinkType = new wxStaticText(this, wxID_ANY, "链接类型:"); 
	wxArrayString stringLinkType;
	stringLinkType.Add("可见矩形");
	stringLinkType.Add("不可见矩形");
	m_choiceLinkType = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringLinkType);
	m_choiceLinkType->SetSelection(0);
	sizerLinkType->Add(textLinkType, 0, wxALIGN_CENTER | wxALL, 5);
	sizerLinkType->Add(m_choiceLinkType, 1, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer *sizerHighlight = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* textHighlight = new wxStaticText(this, wxID_ANY, "高亮样式:");
	wxArrayString stringHighlight;
	stringHighlight.Add("无");
	stringHighlight.Add("反色");
	stringHighlight.Add("边框");
	stringHighlight.Add("内陷");
	m_choiceHighlight = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringHighlight);
	m_choiceHighlight->SetSelection(0);
	sizerHighlight->Add(textHighlight, 0, wxALIGN_CENTER | wxALL, 5);
	sizerHighlight->Add(m_choiceHighlight, 1, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer *sizerLineWidth = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* textLineWidth = new wxStaticText(this, wxID_ANY, "线条宽度:");
	wxArrayString stringLineWidth;
	stringLineWidth.Add("窄");
	stringLineWidth.Add("中");
	stringLineWidth.Add("宽");
	m_choiceLineWidth = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringLineWidth);
	m_choiceLineWidth->SetSelection(0);
	sizerLineWidth->Add(textLineWidth, 0, wxALIGN_CENTER | wxALL, 5);
	sizerLineWidth->Add(m_choiceLineWidth, 1, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer *sizerLineStyle = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* textLineStyle = new wxStaticText(this, wxID_ANY, "线条样式:");
	wxArrayString stringLineStyle;
	stringLineStyle.Add("实线");
	stringLineStyle.Add("虚线");
	stringLineStyle.Add("下划线");
	m_choiceLineStyle = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringLineStyle);
	m_choiceLineStyle->SetSelection(0);
	sizerLineStyle->Add(textLineStyle, 0, wxALIGN_CENTER | wxALL, 5);
	sizerLineStyle->Add(m_choiceLineStyle, 1, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer *sizerLinkColor = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* textLinkColor = new wxStaticText(this, wxID_ANY, "线条颜色:");
	m_pickerLink = new wxColourPickerCtrl(this, wxID_ANY);
	sizerLinkColor->Add(textLinkColor, 0, wxALIGN_CENTER | wxALL, 5);
	sizerLinkColor->Add(m_pickerLink, 1, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer *sizerTextColor = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* textTextColor = new wxStaticText(this, wxID_ANY, "文本颜色:");
	m_pickerText = new wxColourPickerCtrl(this, wxID_ANY);
	sizerTextColor->Add(textTextColor, 0, wxALIGN_CENTER | wxALL, 5);
	sizerTextColor->Add(m_pickerText, 1, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer* sizerLinkSettingALL = new wxBoxSizer(wxVERTICAL);
	sizerLinkSettingALL->Add(sizerLinkType, 1, wxALIGN_LEFT | wxALL, 5);
	sizerLinkSettingALL->Add(sizerHighlight, 1, wxALIGN_LEFT | wxALL, 5);
	sizerLinkSettingALL->Add(sizerLineWidth, 1, wxALIGN_LEFT | wxALL, 5);
	sizerLinkSettingALL->Add(sizerLineStyle, 1, wxALIGN_LEFT | wxALL, 5);
	sizerLinkSettingALL->Add(sizerLinkColor, 1, wxALIGN_LEFT | wxALL, 5);
	sizerLinkSettingALL->Add(sizerTextColor, 1, wxALIGN_LEFT | wxALL, 5);
	sizerLinkSetting->Add(sizerLinkSettingALL, 0, wxALIGN_LEFT | wxALL, 5);

	wxStaticBoxSizer *sizerLinkBehavior = new wxStaticBoxSizer(wxVERTICAL, this, "链接行为:");

	wxBoxSizer* sizerMagni = new wxBoxSizer(wxHORIZONTAL);
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
	sizerMagni->Add(statxtMagni, 0, wxALIGN_CENTER | wxALL, 5);
	sizerMagni->Add(m_choiceMagni, 1, wxALIGN_CENTER | wxALL, 5);
	sizerMagni->Add(m_textMagni, 1, wxALIGN_CENTER | wxALL, 5);
	sizerMagni->Add(statxtPercent, 0, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer* sizerOption = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* statxtOption = new wxStaticText(this, wxID_ANY, "请指定打开首选项:");
	wxArrayString stringsOption;
	stringsOption.Add(wxT("用户首选项设置的窗口"));
	stringsOption.Add(wxT("新建窗口"));
	stringsOption.Add(wxT("现有窗口"));
	m_choiceOption = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, stringsOption);
	m_choiceOption->SetSelection(0);
	sizerOption->Add(statxtOption, 0, wxALIGN_CENTER | wxALL, 5);
	sizerOption->Add(m_choiceOption, 1, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer* sizerLinkBehaviorALL = new wxBoxSizer(wxVERTICAL);
	sizerLinkBehaviorALL->Add(sizerMagni, 0, wxALIGN_LEFT | wxALL, 5);
	sizerLinkBehaviorALL->Add(sizerOption, 0, wxALIGN_LEFT | wxALL, 5);
	sizerLinkBehavior->Add(sizerLinkBehaviorALL, 0, wxALIGN_LEFT | wxALL, 5);

	wxBoxSizer *sizerLink = new wxBoxSizer(wxVERTICAL);
	sizerLink->Add(sizerLinkSetting, 7, wxEXPAND | wxALL, 5);
	sizerLink->Add(sizerLinkBehavior, 3, wxEXPAND | wxALL, 5);

	wxBoxSizer *sizerTop = new wxBoxSizer(wxVERTICAL);
	sizerTop->Add(sizerLink, 0, wxALIGN_CENTER | wxALL, 5);

	SetSizer(sizerTop);
	SetAutoLayout(true);
	//Layout();

	//sizerTop->SetSizeHints(this);
	//sizerTop->Fit(this);
}

MyLinkStyle PanelStyle::getLinkStyle()
{
	MyLinkStyle link;
	link.linkType = m_choiceLinkType->GetSelection();
	link.highlight = m_choiceHighlight->GetSelection();
	link.lineStyle = m_choiceLineStyle->GetSelection();
	link.lineWidth = m_choiceLineWidth->GetSelection();
	wxColor linkColor = m_pickerLink->GetColour();
	link.linkColor[0] = linkColor.Red();
	link.linkColor[1] = linkColor.Green();
	link.linkColor[2] = linkColor.Blue();
	wxColor textColor = m_pickerText->GetColour();
	link.textColor[0] = textColor.Red();
	link.textColor[1] = textColor.Green();
	link.textColor[2] = textColor.Blue();

	MyBookmarkAction action;
	float zoomRate;
	action.option = m_choiceOption->GetStringSelection();
	wxString zoomType = m_choiceMagni->GetStringSelection();
	if (zoomType == "缩放") {
		regex r_int("[0-9]+");
		regex r_float("[0-9]+[.][0-9]+");
		string stringZoom = m_textMagni->GetLineText(0);
		if (!regex_match(stringZoom, r_int) && !regex_match(stringZoom, r_float)) {
			wxMessageBox("请输入正确的放大率", "Warning", wxOK | wxICON_WARNING, this);
			link.isValid = false;
		}
		zoomRate = wxAtof(stringZoom) / 100;
		if (zoomRate <= 0) {
			wxMessageBox("请输入正确的放大率", "Warning", wxOK | wxICON_WARNING, this);
			link.isValid = false;
		}
	}
	action.zoomType = zoomType;
	action.zoomRate = zoomRate;

	link.action = action;

	return link;
}

void PanelStyle::OnChoiceANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_choiceMagni)
	{
		if (m_choiceMagni->GetStringSelection() == "缩放")
			m_textMagni->Enable(true);
		else
			m_textMagni->Enable(false);
	}
	//else if (event.GetEventObject() == m_choiceLinkType) {
	//	if (m_choiceLinkType->GetStringSelection() == "不可见矩形") {
	//		m_choiceLineStyle->Enable(false);
	//		m_choiceLineWidth->Enable(false);
	//		m_choiceLinkColor->Enable(false);
	//	}
	//	else {
	//		m_choiceLineStyle->Enable(true);
	//		m_choiceLineWidth->Enable(true);
	//		m_choiceLinkColor->Enable(true);
	//	}
	//}
	else
	{
		event.Skip();
	}
}

BEGIN_EVENT_TABLE(PanelStyle, wxScrolled)
EVT_CHOICE(wxID_ANY, PanelStyle::OnChoiceANY)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// AddLinkAnnotationDialog
// ----------------------------------------------------------------------------
AddLinkAnnotationDialog::AddLinkAnnotationDialog(wxWindow *parent)
	: wxDialog(parent, wxID_ANY, wxString(_T("添加链接")), wxDefaultPosition, wxSize(400, 500), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX)
{
	////this->SetWindowStyleFlag(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);
	wxBoxSizer *sizerConfig = new wxBoxSizer(wxHORIZONTAL);
	m_choiceProp = new wxChoice(this, wxID_ANY);
	m_btnSave = new wxButton(this, wxID_ANY, "保存配置");
	m_btnDelete = new wxButton(this, wxID_ANY, "删除配置");
	sizerConfig->Add(m_choiceProp, 1, wxALIGN_CENTER | wxALL, 5);
	sizerConfig->Add(m_btnSave, 1, wxALIGN_CENTER | wxALL, 5);
	sizerConfig->Add(m_btnDelete, 1, wxALIGN_CENTER | wxALL, 5);

	wxNotebook *notebook = new wxNotebook(this, wxID_ANY);
	m_panelStyle = new PanelStyle(notebook);
	m_panelDest = new PanelDest(notebook);
	notebook->AddPage(m_panelDest, "目标");
	notebook->AddPage(m_panelStyle, "链接属性");

	wxBoxSizer *sizerBtn = new wxBoxSizer(wxHORIZONTAL);
	wxButton *btnOk = new wxButton(this, wxID_OK, "设置链接");
	wxButton *btnCancel = new wxButton(this, wxID_CANCEL, "关闭");
	sizerBtn->Add(btnOk, 1, wxALIGN_CENTER | wxALL, 5);
	sizerBtn->Add(btnCancel, 1, wxALIGN_CENTER | wxALL, 5);

	sizerTop = new wxBoxSizer(wxVERTICAL);
	sizerTop->Add(sizerConfig, 1, wxALIGN_CENTER | wxALL, 5);
	sizerTop->Add(notebook, 10, wxALIGN_CENTER | wxALL, 5);
	sizerTop->Add(sizerBtn, 1, wxALIGN_RIGHT | wxALL, 5);

	//SetSizerAndFit(sizerTop);
	SetSizer(sizerTop);
	SetAutoLayout(true);
	Layout();

	sizerTop->SetSizeHints(this);
	sizerTop->Fit(this);
	//sizerTop->FitInside(this);

	btnCancel->SetFocus();
	btnCancel->SetDefault();
}

void AddLinkAnnotationDialog::OnButtonANY(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_btnSave) {

	}
	else
	{
		event.Skip();
	}
}

void AddLinkAnnotationDialog::OnButtonOK(wxCommandEvent& event)
{
	MyLinkStyle style = m_panelStyle->getLinkStyle();
	MyLinkDest dest = m_panelDest->getDest();
	if (!dest.isValid || !style.isValid)
		return;

	dest.action.option = style.action.option;
	dest.action.zoomRate = style.action.zoomRate;
	dest.action.zoomType = style.action.zoomType;

	m_linkAnnotation.dest = dest;
	m_linkAnnotation.style = style;

	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(AddLinkAnnotationDialog, wxDialog)
EVT_BUTTON(wxID_ANY, AddLinkAnnotationDialog::OnButtonANY)
EVT_BUTTON(wxID_OK, AddLinkAnnotationDialog::OnButtonOK)
END_EVENT_TABLE()


GetBookmarkDialog::GetBookmarkDialog(wxWindow *parent, MTree* bookmarks)
	: wxDialog(parent, wxID_ANY, "")
{
	m_tree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(400, 600), wxTR_HIDE_ROOT | wxTR_DEFAULT_STYLE);
	m_rootId = m_tree->AddRoot("书签");
	SetOriginBookmarks(bookmarks);

	wxButton *btnOk = new wxButton(this, wxID_OK, "确定");
	wxButton *btnCancel = new wxButton(this, wxID_CANCEL, "取消");
	wxBoxSizer* sizerBtn = new wxBoxSizer(wxHORIZONTAL);
	sizerBtn->Add(btnOk, 0, wxALIGN_CENTER | wxALL, 5);
	sizerBtn->Add(btnCancel, 0, wxALIGN_CENTER | wxALL, 5);

	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	sizerTop->Add(m_tree, 0, wxALIGN_CENTER | wxALL, 10);
	sizerTop->Add(sizerBtn, 0, wxALIGN_CENTER | wxALL, 10);

	SetSizer(sizerTop);

	sizerTop->SetSizeHints(this);
	sizerTop->Fit(this);

	btnOk->SetFocus();
	btnOk->SetDefault();
}

void GetBookmarkDialog::VisitAllNodes(MNode* node, wxTreeItemId parentId)
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

void GetBookmarkDialog::SetOriginBookmarks(MTree* tree)
{
	MNode* root = tree->getRoot();
	VisitAllNodes(root, m_rootId);
}

void GetBookmarkDialog::OnButtonOK(wxCommandEvent& event)
{
	wxTreeItemId itemId = m_tree->GetSelection();
	if (!itemId.IsOk()) {
		wxMessageBox("请选择书签", "Warning", wxOK | wxICON_WARNING, this);
		return;
	}
	MyTreeItemData* itemData = (MyTreeItemData*)m_tree->GetItemData(itemId);
	m_bookmark = itemData->GetBookmark();

	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(GetBookmarkDialog, wxDialog)
EVT_BUTTON(wxID_OK, GetBookmarkDialog::OnButtonOK)
END_EVENT_TABLE()