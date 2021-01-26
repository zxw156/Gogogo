#ifndef __DIALOGSH__
#define __DIALOGSH__

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

#include <vector>
#include <map>

#include "mytree.h"

// BookmarkManagerDialog
class BookmarkManagerDialog : public wxDialog
{
public:
    BookmarkManagerDialog(wxWindow *parent);

	void SetOriginBookmarks(MTree* tree);
	void SetOriginNamedDests(wxArrayString names);
    std::vector<MyBookmark> GetBookmarks();

private:
	wxArrayString m_nameDests;
    std::vector<MyBookmark> m_bookmarks;
	wxTreeItemId m_rootId;
	wxTreeCtrl *m_tree;
	wxButton *m_btnAdd,
			 *m_btnEdit,
             *m_btnRemove,
             *m_btnImport;
	wxChoice *m_choiceInsertOption;

	void OnButtonANY(wxCommandEvent& event);

	vector<int> ItemIdToIndexArray(wxTreeItemId itemId);
	void VisitAllNodes(MNode* node, wxTreeItemId parentId);

    DECLARE_EVENT_TABLE()
};

class AddBookmarkDialog : public wxDialog
{
public:
    AddBookmarkDialog(wxWindow *parent, MyBookmark bookmark);

    MyBookmark GetBookmark();

private:
    MyBookmark m_bookmark;
    MyBookmarkAction m_action;
    wxTextCtrl *m_textctrl;
	wxColourPickerCtrl *m_colorPicker;
    wxChoice *m_choiceStyle,
            *m_choiceAction;
    wxButton *m_btnEditAction,
            *m_btnOk;

	void OnButtonANY(wxCommandEvent& event);
	void OnButtonOK(wxCommandEvent& event);
	void OnChoiceANY(wxCommandEvent& event);
	void OnTextANY(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};

class OpenAFileViewDialog : public wxDialog
{
public:
	OpenAFileViewDialog(wxWindow *parent, wxString title, MyBookmarkAction action);

	MyBookmarkAction GetAction();

private:
	int m_pageNum;
	MyBookmarkAction m_action;
	wxTextCtrl *m_textFile;
	wxStaticText *m_textNamedDest, *m_textPageNum;
	wxTextCtrl	*m_textMagni,
		*m_textPage;
	wxChoice *m_choiceMagni,
		*m_choiceOption;
	wxButton *m_btnChooseFile,
		*m_btnChooseNamedDest,
		*m_btnOk;
	wxRadioButton *m_rbtnPage,
		*m_rbtnNamedDest;

	void OnButtonANY(wxCommandEvent& event);
	void OnButtonOK(wxCommandEvent& event);
	void OnChoiceANY(wxCommandEvent& event);
	void OnRadiobuttonANY(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

class OpenFileOptionDialog : public wxDialog
{
public:
	OpenFileOptionDialog(wxWindow *parent);
	wxString GetOption() { return m_option; }
private:
	wxRadioBox *m_radioBox;
	wxString m_option;
	void OnButtonOK(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

class GetNamedDestDialog : public wxDialog
{
public:
	GetNamedDestDialog(wxWindow *parent, wxArrayString names);
	wxString GetName() { return m_name; }
private:
	wxListBox *m_list;
	wxString m_name;
	void OnButtonOK(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

class ImportBookmarkDialog : public wxDialog
{
public:
	ImportBookmarkDialog(wxWindow *parent);
private:
	wxRadioButton *m_rbtnPage,
		*m_rbtnRange;
	wxChoice *m_choicePage,
		*m_choiceSeperator;
	wxTextCtrl *m_textRange,
		*m_textSeperator;
	wxCheckBox *m_ckboxSeperator,
		*m_ckboxNumber;
	void OnButtonOK(wxCommandEvent& event);
	void OnRadiobuttonANY(wxCommandEvent& event);
	void OnChoiceANY(wxCommandEvent& event);
	void OnCheckBoxANY(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};
#endif