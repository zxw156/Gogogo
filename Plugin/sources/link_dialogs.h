#ifndef __LINK_DIALOGSH__
#define __LINK_DIALOGSH__

#include "wx/dialog.h"
#include "wx/button.h"
#include "wx/listbox.h"
#include "wx/choice.h"
#include "wx/treectrl.h"
#include "wx/radiobox.h"
#include "wx/radiobut.h"
#include "wx/stattext.h"
#include "wx/clrpicker.h"
#include "wx/checkbox.h"
#include "wx/notebook.h"
#include "wx/scrolwin.h"
#include "wx/spinctrl.h"

#include "bookmark.h"
#include "mytree.h"

#include <vector>
#include <map>

typedef struct MyLinkDest
{
	bool isValid = true;
	bool useBookmark;
	MyBookmarkAction action;
	PDAction bookmarkAction;
}MyLinkDest;

typedef struct MyLinkStyle
{
	bool isValid = true;

	int linkType;
	int highlight;
	int lineWidth;
	int lineStyle;
	int linkColor[3] = { 0 };
	int textColor[3] = { 0 };

	MyBookmarkAction action;
}MyLinkStyle;

typedef struct MyLinkAnnotation
{
	MyLinkDest dest;
	MyLinkStyle style;
}MyLinkAnnotation;

class GetBookmarkDialog : public wxDialog
{
public:
	GetBookmarkDialog(wxWindow *parent, MTree* bookmarks);
	MyBookmark GetBookmark() { return m_bookmark; }
	void SetOriginBookmarks(MTree* tree);

private:
	wxTreeCtrl *m_tree;
	wxTreeItemId m_rootId;
	MyBookmark m_bookmark;
	void OnButtonOK(wxCommandEvent& event);
	void VisitAllNodes(MNode* node, wxTreeItemId parentId);

	DECLARE_EVENT_TABLE()
};

class PanelDest : public wxScrolled<wxPanel>
{
public:
	PanelDest(wxWindow *parent);
	MyLinkDest getDest();

private:
	wxButton *m_btnFile,
		*m_btnBookmark,
		*m_btnDelete;
	wxRadioButton *m_rbtnFirstPage,
		*m_rbtnPage,
		*m_rbtnNamedDest,
		*m_rbtnBookmark;
	wxChoice *m_choiceLinkType,
		*m_choiceNamedDest;
	wxTextCtrl* m_textFilePath;
	wxSpinCtrl* m_spinPage;
	wxStaticText *m_textPageNum,
		*m_textBookmark;
	int m_currentPageNum;
	wxArrayString m_currentNamedDest;
	MTree* m_currentBookmark,
		*m_fileBookmark;
	MyBookmark m_bookmark;

	void OnButtonANY(wxCommandEvent& event);
	void OnChoiceANY(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

class PanelStyle : public wxScrolled<wxPanel>
{
public:
	PanelStyle(wxWindow *parent);
	MyLinkStyle getLinkStyle();
	void setAction(MyBookmarkAction* action);

private:
	wxChoice *m_choiceLinkType,
		*m_choiceHighlight,
		*m_choiceLineWidth,
		*m_choiceLineStyle,
		*m_choiceLinkColor,
		*m_choiceTextColor,
		*m_choiceMagni,
		*m_choiceOption;
	wxColourPickerCtrl* m_pickerLink,
		*m_pickerText;
	wxTextCtrl *m_textMagni;


	void OnChoiceANY(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

// AddLinkAnnotationDialog
class AddLinkAnnotationDialog : public wxDialog
{
public:
	AddLinkAnnotationDialog(wxWindow *parent);
	MyLinkAnnotation getLinkAnnotation() { return m_linkAnnotation; };

private:
	wxBoxSizer * sizerTop;
	wxButton *m_btnSave,
		*m_btnDelete;
	wxChoice *m_choiceProp;
	PanelStyle *m_panelStyle;
	PanelDest *m_panelDest;
	MyLinkAnnotation m_linkAnnotation;

	void OnButtonANY(wxCommandEvent& event);
	void OnButtonOK(wxCommandEvent& event);
	//void OnSize(wxSizeEvent& event);
    
	DECLARE_EVENT_TABLE()
};

#endif