//////////////////////////////////////////////////////////////////////////////
// File:        contrib/samples/stc/stctest.cpp
// Purpose:     STC test application
// Maintainer:  Otto Wyss
// Created:     2003-09-01
// Copyright:   (c) wxGuide
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// headers
//----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all 'standard' wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

//! wxWidgets headers
#include "wx/config.h"   // configuration support
#include "wx/filedlg.h"  // file dialog support
#include "wx/filename.h" // filename support
#include "wx/notebook.h" // notebook support
#include "wx/settings.h" // system settings
#include "wx/string.h"   // strings support
#include "wx/image.h"    // images support
#include <wx/aui/auibook.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>

//! application headers
#include "defsext.h"     // Additional definitions
#include "edit.h"        // Edit module
#include "dialogs.h"        // Edit module
#include "prefs.h"       // Prefs

//----------------------------------------------------------------------------
// resources
//----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources)
#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "sample.xpm"
#endif

//============================================================================
// declarations
//============================================================================

#define APP_NAME wxT("STC-Test")
#define APP_DESCR _("See http://wxguide.sourceforge.net/")

#define APP_MAINT wxT("Otto Wyss")
#define APP_VENDOR wxT("wxWidgets")
#define APP_COPYRIGTH wxT("(C) 2003 Otto Wyss")
#define APP_LICENCE wxT("wxWidgets")

#define APP_VERSION wxT("0.1.alpha")
#define APP_BUILD __DATE__

#define APP_WEBSITE wxT("http://www.wxWidgets.org")
#define APP_MAIL wxT("mailto://???")

#define NONAME _("<untitled>")

class AppBook;


//----------------------------------------------------------------------------
//! global application name
wxString *g_appname = NULL;

#if wxUSE_PRINTING_ARCHITECTURE

//! global print data, to remember settings during the session
wxPrintData *g_printData = (wxPrintData*) NULL;
wxPageSetupDialogData *g_pageSetupData = (wxPageSetupDialogData*) NULL;

#endif // wxUSE_PRINTING_ARCHITECTURE


class AppFrame;

//----------------------------------------------------------------------------
//! application APP_VENDOR-APP_NAME.
class App: public wxApp {
    friend class AppFrame;

public:
    //! the main function called durning application start
    virtual bool OnInit ();

    //! application exit function
    virtual int OnExit ();

private:
    //! frame window
    AppFrame* m_frame;

protected:
    DECLARE_EVENT_TABLE()
};

// created dynamically by wxWidgets
DECLARE_APP (App);

//----------------------------------------------------------------------------
//! frame of the application APP_VENDOR-APP_NAME.
class AppFrame: public wxFrame {
    friend class App;
    friend class AppBook;
    friend class AppAbout;

public:
    //! constructor
    AppFrame (const wxString &title);

    //! destructor
    ~AppFrame ();

    //! event handlers
    //! common
    void OnClose (wxCloseEvent &event);
    void OnAbout (wxCommandEvent &event);
    void OnExit (wxCommandEvent &event);
    void OnTimerEvent (wxTimerEvent &event);
    //! file
    void OnFileNew (wxCommandEvent &event);
    void OnFileNewFrame (wxCommandEvent &event);
    void OnFileOpen (wxCommandEvent &event);
    void OnFileOpenFrame (wxCommandEvent &event);
    void OnFileSave (wxCommandEvent &event);
    void OnFileSaveAs (wxCommandEvent &event);
    void OnFileClose (wxCommandEvent &event);
    //! properties
    void OnProperties (wxCommandEvent &event);
    //! print
    void OnPrintSetup (wxCommandEvent &event);
    void OnPrintPreview (wxCommandEvent &event);
    void OnPrint (wxCommandEvent &event);
    //! edit events
    void OnEdit (wxCommandEvent &event);
    void OnFindReplace(wxFindDialogEvent& event);

    void OnNotebook(wxAuiNotebookEvent& event);

private:
    // edit object
    Edit *m_edit;
    wxAuiNotebook* m_notebook;
    void FileNew(Edit* edit);
    void FileOpen (wxString fname, Edit* edit);

    //! creates the application menu bar
    wxMenuBar *m_menuBar;
    void CreateMenu ();

    // print preview position and size
    wxRect DeterminePrintSize ();

    DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
//! about box of the application APP_VENDOR-APP_NAME
class AppAbout: public wxDialog {

public:
    //! constructor
    AppAbout (wxWindow *parent,
              int milliseconds = 0,
              long style = 0);

    //! destructor
    ~AppAbout ();

    // event handlers
    void OnTimerEvent (wxTimerEvent &event);

private:
    // timer
    wxTimer *m_timer;

    DECLARE_EVENT_TABLE()
};


//============================================================================
// implementation
//============================================================================

IMPLEMENT_APP (App)


BEGIN_EVENT_TABLE(App, wxApp)
END_EVENT_TABLE()

//----------------------------------------------------------------------------
// App
//----------------------------------------------------------------------------

bool App::OnInit () {

    wxInitAllImageHandlers();

    // set application and vendor name
    SetAppName (APP_NAME);
    SetVendorName (APP_VENDOR);
    g_appname = new wxString ();
    g_appname->Append (APP_VENDOR);
    g_appname->Append (wxT("-"));
    g_appname->Append (APP_NAME);

#if wxUSE_PRINTING_ARCHITECTURE
    // initialize print data and setup
    g_printData = new wxPrintData;
    g_pageSetupData = new wxPageSetupDialogData;
#endif // wxUSE_PRINTING_ARCHITECTURE

    // create application frame
    m_frame = new AppFrame (*g_appname);

    // open application frame
    m_frame->Layout ();
    m_frame->Show (true);

    return true;
}

int App::OnExit () {

    // delete global appname
    delete g_appname;

#if wxUSE_PRINTING_ARCHITECTURE
    // delete global print data and setup
    if (g_printData) delete g_printData;
    if (g_pageSetupData) delete g_pageSetupData;
#endif // wxUSE_PRINTING_ARCHITECTURE

    return 0;
}

//----------------------------------------------------------------------------
// AppFrame
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (AppFrame, wxFrame)
    // common
    EVT_CLOSE (                      AppFrame::OnClose)
    // file
    EVT_MENU (wxID_NEW,             AppFrame::OnFileNew)
    EVT_MENU (wxID_OPEN,             AppFrame::OnFileOpen)
    EVT_MENU (wxID_SAVE,             AppFrame::OnFileSave)
    EVT_MENU (wxID_SAVEAS,           AppFrame::OnFileSaveAs)
    EVT_MENU (wxID_CLOSE,            AppFrame::OnFileClose)
    // properties
    EVT_MENU (myID_PROPERTIES,       AppFrame::OnProperties)
    // print and exit
    EVT_MENU (wxID_PRINT_SETUP,      AppFrame::OnPrintSetup)
    EVT_MENU (wxID_PREVIEW,          AppFrame::OnPrintPreview)
    EVT_MENU (wxID_PRINT,            AppFrame::OnPrint)
    EVT_MENU (wxID_EXIT,             AppFrame::OnExit)
    // Menu items with standard IDs forwarded to the editor.
    EVT_MENU (wxID_CLEAR,            AppFrame::OnEdit)
    EVT_MENU (wxID_CUT,              AppFrame::OnEdit)
    EVT_MENU (wxID_COPY,             AppFrame::OnEdit)
    EVT_MENU (wxID_PASTE,            AppFrame::OnEdit)
    EVT_MENU (wxID_SELECTALL,        AppFrame::OnEdit)
    EVT_MENU (wxID_REDO,             AppFrame::OnEdit)
    EVT_MENU (wxID_UNDO,             AppFrame::OnEdit)
    EVT_MENU (wxID_FIND,             AppFrame::OnEdit)
    // And all our edit-related menu commands.
    EVT_MENU_RANGE (myID_EDIT_FIRST, myID_EDIT_LAST,
                                     AppFrame::OnEdit)
    // help
    EVT_MENU (wxID_ABOUT,            AppFrame::OnAbout)

    // find dialog
    EVT_FIND(wxID_ANY, AppFrame::OnFindReplace)
    EVT_FIND_NEXT(wxID_ANY, AppFrame::OnFindReplace)
    EVT_FIND_REPLACE(wxID_ANY, AppFrame::OnFindReplace)
    EVT_FIND_REPLACE_ALL(wxID_ANY, AppFrame::OnFindReplace)
    EVT_FIND_CLOSE(wxID_ANY, AppFrame::OnFindReplace)

    EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, AppFrame::OnNotebook)
    EVT_AUINOTEBOOK_PAGE_CHANGING(wxID_ANY, AppFrame::OnNotebook)
END_EVENT_TABLE ()

AppFrame::AppFrame (const wxString &title)
        : wxFrame ((wxFrame *)NULL, wxID_ANY, title, wxDefaultPosition, wxSize(750,550),
                    wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    SetIcon(wxICON(sample));

    // initialize important variables
    m_edit = NULL;

    // set icon and background
    SetTitle (*g_appname);
    SetBackgroundColour (wxT("WHITE"));

    // create menu
    m_menuBar = new wxMenuBar;
    CreateMenu ();

    m_notebook = new wxAuiNotebook(this, 1);

    m_edit = new Edit(m_notebook, 0);
    m_edit->SetFocus();

    m_notebook->InsertPage(0, m_edit, wxT("base64.obs"));



    Edit* foo = new Edit(m_notebook, 3);
    foo->SetFocus();
    m_notebook->InsertPage(1, foo, wxT("foo.obs"));

//    FileOpen (wxT("C:\\Users\\Randy\\Documents\\Code\\objeck\\main\\src\\compiler\\test_src\\debug.obs"));
    FileOpen (wxT("/Users/randy/Documents/Code/objeck/main/src/compiler/rc/base64.obs"), m_edit);
    FileOpen(wxT("/Users/randy/Documents/Code/objeck/main/src/compiler/rc/hello.obs"), foo);
}

AppFrame::~AppFrame () {
}

// common event handlers
void AppFrame::OnNotebook(wxAuiNotebookEvent& event) {
  const wxEventType eventType = event.GetEventType();
  if(eventType == wxEVT_AUINOTEBOOK_PAGE_CHANGED) {
    m_edit = (Edit*)m_notebook->GetCurrentPage();
  }
}

void AppFrame::OnClose (wxCloseEvent &event) {
    wxCommandEvent evt;
    OnFileClose (evt);
    if (m_edit && m_edit->Modified()) {
        if (event.CanVeto()) event.Veto (true);
        return;
    }
    Destroy();
}

void AppFrame::OnAbout (wxCommandEvent &WXUNUSED(event)) {
    AppAbout dlg(this);
}

void AppFrame::OnExit (wxCommandEvent &WXUNUSED(event)) {
    Close (true);
}

// file event handlers
void AppFrame::OnFileOpen (wxCommandEvent &WXUNUSED(event)) {
    if (!m_edit) return;
#if wxUSE_FILEDLG
    wxString fname;
    wxFileDialog dlg (this, wxT("Open file"), wxEmptyString, wxEmptyString, wxT("Any file (*)|*"),
                      wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
    if (dlg.ShowModal() != wxID_OK) return;
    fname = dlg.GetPath ();
    FileOpen (fname, m_edit);
#endif // wxUSE_FILEDLG
}

void AppFrame::OnFileSave (wxCommandEvent &WXUNUSED(event)) {
    if (!m_edit) return;
    if (!m_edit->Modified()) {
        wxMessageBox (_("There is nothing to save!"), _("Save file"),
                      wxOK | wxICON_EXCLAMATION);
        return;
    }
    m_edit->SaveFile ();
}

void AppFrame::OnFileSaveAs (wxCommandEvent &WXUNUSED(event)) {
    if (!m_edit) return;
#if wxUSE_FILEDLG
    wxString filename = wxEmptyString;
    wxFileDialog dlg (this, wxT("Save file"), wxEmptyString, wxEmptyString, wxT("Any file (*)|*"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() != wxID_OK) return;
    filename = dlg.GetPath();
    m_edit->SaveFile (filename);
#endif // wxUSE_FILEDLG
}

void AppFrame::OnFileClose (wxCommandEvent &WXUNUSED(event)) {
    if (!m_edit) return;
    if (m_edit->Modified()) {
        if (wxMessageBox (_("Text is not saved, save before closing?"), _("Close"),
                          wxYES_NO | wxICON_QUESTION) == wxYES) {
            m_edit->SaveFile();
            if (m_edit->Modified()) {
                wxMessageBox (_("Text could not be saved!"), _("Close abort"),
                              wxOK | wxICON_EXCLAMATION);
                return;
            }
        }
    }
    m_edit->SetFilename (wxEmptyString);
    m_edit->ClearAll();
    m_edit->SetSavePoint();
}

// properties event handlers
void AppFrame::OnProperties (wxCommandEvent &WXUNUSED(event)) {
    if (!m_edit) return;
    EditProperties dlg(m_edit, 0);
}

// print event handlers
void AppFrame::OnPrintSetup (wxCommandEvent &WXUNUSED(event)) {
#if wxUSE_PRINTING_ARCHITECTURE
    (*g_pageSetupData) = * g_printData;
    wxPageSetupDialog pageSetupDialog(this, g_pageSetupData);
    pageSetupDialog.ShowModal();
    (*g_printData) = pageSetupDialog.GetPageSetupData().GetPrintData();
    (*g_pageSetupData) = pageSetupDialog.GetPageSetupData();
#endif // wxUSE_PRINTING_ARCHITECTURE
}

void AppFrame::OnPrintPreview (wxCommandEvent &WXUNUSED(event)) {
#if wxUSE_PRINTING_ARCHITECTURE
    wxPrintDialogData printDialogData( *g_printData);
    wxPrintPreview *preview =
        new wxPrintPreview (new EditPrint (m_edit),
                            new EditPrint (m_edit),
                            &printDialogData);
    if (!preview->IsOk()) {
        delete preview;
        wxMessageBox (_("There was a problem with previewing.\n\
                         Perhaps your current printer is not correctly?"),
                      _("Previewing"), wxOK);
        return;
    }
    wxRect rect = DeterminePrintSize();
    wxPreviewFrame *frame = new wxPreviewFrame (preview, this, _("Print Preview"));
    frame->SetSize (rect);
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show(true);
#endif // wxUSE_PRINTING_ARCHITECTURE
}

void AppFrame::OnPrint (wxCommandEvent &WXUNUSED(event)) {
#if wxUSE_PRINTING_ARCHITECTURE
    wxPrintDialogData printDialogData( *g_printData);
    wxPrinter printer (&printDialogData);
    EditPrint printout (m_edit);
    if (!printer.Print (this, &printout, true)) {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
        wxMessageBox (_("There was a problem with printing.\n\
                         Perhaps your current printer is not correctly?"),
                      _("Previewing"), wxOK);
            return;
        }
    }
    (*g_printData) = printer.GetPrintDialogData().GetPrintData();
#endif // wxUSE_PRINTING_ARCHITECTURE
}

// edit events
void AppFrame::OnEdit (wxCommandEvent &event) {
    if (m_edit) m_edit->GetEventHandler()->ProcessEvent (event);
}

// edit events
void AppFrame::OnFindReplace(wxFindDialogEvent& event) {
  if(m_edit) m_edit->GetEventHandler()->ProcessEvent(event);
}

// private functions
void AppFrame::CreateMenu ()
{
    // File menu
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_NEW, _("&New ..\tCtrl+N"));
    menuFile->Append (wxID_OPEN, _("&Open ..\tCtrl+O"));
    menuFile->Append (wxID_SAVE, _("&Save\tCtrl+S"));
    menuFile->Append (wxID_SAVEAS, _("Save &as ..\tCtrl+Shift+S"));
    menuFile->Append (wxID_CLOSE, _("&Close\tCtrl+W"));
    menuFile->AppendSeparator();
    menuFile->Append (myID_PROPERTIES, _("Proper&ties ..\tCtrl+I"));
    menuFile->AppendSeparator();
    menuFile->Append (wxID_PRINT_SETUP, _("Print Set&up .."));
    menuFile->Append (wxID_PREVIEW, _("Print Pre&view\tCtrl+Shift+P"));
    menuFile->Append (wxID_PRINT, _("&Print ..\tCtrl+P"));
    menuFile->AppendSeparator();
    menuFile->Append (wxID_EXIT, _("&Quit\tCtrl+Q"));

    // Edit menu
    wxMenu *menuEdit = new wxMenu;
    menuEdit->Append (wxID_UNDO, _("&Undo\tCtrl+Z"));
    menuEdit->Append (wxID_REDO, _("&Redo\tCtrl+Shift+Z"));
    menuEdit->AppendSeparator();
    menuEdit->Append (wxID_CUT, _("Cu&t\tCtrl+X"));
    menuEdit->Append (wxID_COPY, _("&Copy\tCtrl+C"));
    menuEdit->Append (wxID_PASTE, _("&Paste\tCtrl+V"));
    menuEdit->Append (wxID_CLEAR, _("&Delete\tDel"));
    menuEdit->AppendSeparator();
    menuEdit->Append (wxID_FIND, _("&Find\tCtrl+F"));
    // menuEdit->Enable (wxID_FIND, false);
    menuEdit->Append (myID_FINDNEXT, _("Find &next\tF3"));
    menuEdit->Enable (myID_FINDNEXT, false);
    menuEdit->Append (myID_REPLACE, _("&Replace\tCtrl+H"));
    // menuEdit->Enable (myID_REPLACE, false);
    menuEdit->Append (myID_REPLACENEXT, _("Replace &again\tShift+F4"));
    menuEdit->Enable (myID_REPLACENEXT, false);
    menuEdit->AppendSeparator();
    menuEdit->Append (myID_BRACEMATCH, _("&Match brace\tCtrl+M"));
    menuEdit->Append (myID_GOTO, _("&Goto\tCtrl+G"));
    // menuEdit->Enable (myID_GOTO, false);
    menuEdit->AppendSeparator();
    menuEdit->Append (myID_INDENTINC, _("&Indent increase\tTab"));
    menuEdit->Append (myID_INDENTRED, _("I&ndent reduce\tShift+Tab"));
    menuEdit->AppendSeparator();
    menuEdit->Append (wxID_SELECTALL, _("&Select all\tCtrl+A"));
    menuEdit->Append (myID_SELECTLINE, _("Select &line\tCtrl+L"));

    // hilight submenu
    wxMenu *menuHilight = new wxMenu;
    int Nr;
    for (Nr = 0; Nr < g_LanguagePrefsSize; Nr++) {
        menuHilight->Append (myID_HILIGHTFIRST + Nr,
                             g_LanguagePrefs [Nr].name);
    }

    // charset submenu
    wxMenu *menuCharset = new wxMenu;
    menuCharset->Append (myID_CHARSETANSI, _("&ANSI (Windows)"));
    menuCharset->Append (myID_CHARSETMAC, _("&MAC (Macintosh)"));

    // View menu
    wxMenu *menuView = new wxMenu;
    menuView->Append (myID_HILIGHTLANG, _("&Hilight language .."), menuHilight);
    menuView->AppendSeparator();
    menuView->AppendCheckItem (myID_FOLDTOGGLE, _("&Toggle current fold\tCtrl+T"));
    menuView->AppendCheckItem (myID_OVERTYPE, _("&Overwrite mode\tIns"));
    menuView->AppendCheckItem (myID_WRAPMODEON, _("&Wrap mode\tCtrl+U"));
    menuView->AppendSeparator();
    menuView->AppendCheckItem (myID_DISPLAYEOL, _("Show line &endings"));
    menuView->AppendCheckItem (myID_INDENTGUIDE, _("Show &indent guides"));
    wxMenuItem* showLines = menuView->AppendCheckItem(myID_LINENUMBER, _("Show line &numbers"));
    showLines->Toggle();
    menuView->AppendCheckItem (myID_LONGLINEON, _("Show &long line marker"));
    menuView->AppendCheckItem (myID_WHITESPACE, _("Show white&space"));
    menuView->AppendSeparator();
    menuView->Append (myID_USECHARSET, _("Use &code page of .."), menuCharset);

    // change case submenu
    wxMenu *menuChangeCase = new wxMenu;
    menuChangeCase->Append (myID_CHANGEUPPER, _("&Upper case"));
    menuChangeCase->Append (myID_CHANGELOWER, _("&Lower case"));

    // convert EOL submenu
    wxMenu *menuConvertEOL = new wxMenu;
    menuConvertEOL->Append (myID_CONVERTCR, _("CR (&Linux)"));
    menuConvertEOL->Append (myID_CONVERTCRLF, _("CR+LF (&Windows)"));
    menuConvertEOL->Append (myID_CONVERTLF, _("LF (&Macintosh)"));

    // Extra menu
    wxMenu *menuExtra = new wxMenu;
    menuExtra->AppendCheckItem (myID_READONLY, _("&Readonly mode"));
    menuExtra->AppendSeparator();
    menuExtra->Append (myID_CHANGECASE, _("Change &case to .."), menuChangeCase);
    menuExtra->AppendSeparator();
    menuExtra->Append (myID_CONVERTEOL, _("Convert line &endings to .."), menuConvertEOL);

    // Window menu
    wxMenu *menuWindow = new wxMenu;
    menuWindow->Append (myID_PAGEPREV, _("&Previous\tCtrl+Shift+Tab"));
    menuWindow->Append (myID_PAGENEXT, _("&Next\tCtrl+Tab"));

    // Help menu
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append (wxID_ABOUT, _("&About ..\tCtrl+D"));

    // construct menu
    m_menuBar->Append (menuFile, _("&File"));
    m_menuBar->Append (menuEdit, _("&Edit"));
    m_menuBar->Append (menuView, _("&View"));
    m_menuBar->Append (menuExtra, _("E&xtra"));
    m_menuBar->Append (menuWindow, _("&Window"));
    m_menuBar->Append (menuHelp, _("&Help"));
    SetMenuBar (m_menuBar);
}

void AppFrame::OnFileNew(wxCommandEvent &WXUNUSED(event))
{
  Edit* foo = new Edit(m_notebook, 4);
  foo->SetFocus();
  foo->InitializePrefs("Objeck");
  m_notebook->InsertPage(3, foo, wxT("Untitled*"));
  m_edit = foo;
}

void AppFrame::FileOpen (wxString fname, Edit* edit)
{
    wxFileName w(fname); w.Normalize(); fname = w.GetFullPath();
    edit->LoadFile(fname);
    edit->SelectNone();
}

wxRect AppFrame::DeterminePrintSize () {

    wxSize scr = wxGetDisplaySize();

    // determine position and size (shifting 16 left and down)
    wxRect rect = GetRect();
    rect.x += 16;
    rect.y += 16;
    rect.width = wxMin (rect.width, (scr.x - rect.x));
    rect.height = wxMin (rect.height, (scr.x - rect.y));

    return rect;
}

//----------------------------------------------------------------------------
// AppAbout
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (AppAbout, wxDialog)
    EVT_TIMER (myID_ABOUTTIMER, AppAbout::OnTimerEvent)
END_EVENT_TABLE ()

AppAbout::AppAbout (wxWindow *parent,
                    int milliseconds,
                    long style)
        : wxDialog (parent, wxID_ANY, wxEmptyString,
                    wxDefaultPosition, wxDefaultSize,
                    style | wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {

    // set timer if any
    m_timer = NULL;
    if (milliseconds > 0) {
        m_timer = new wxTimer (this, myID_ABOUTTIMER);
        m_timer->Start (milliseconds, wxTIMER_ONE_SHOT);
    }

    // sets the application title
    SetTitle (_("About .."));

    // about info
    wxGridSizer *aboutinfo = new wxGridSizer (2, 0, 2);
    aboutinfo->Add (new wxStaticText(this, wxID_ANY, _("Written by: ")),
                    0, wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, wxID_ANY, APP_MAINT),
                    1, wxEXPAND | wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, wxID_ANY, _("Version: ")),
                    0, wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, wxID_ANY, APP_VERSION),
                    1, wxEXPAND | wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, wxID_ANY, _("Licence type: ")),
                    0, wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, wxID_ANY, APP_LICENCE),
                    1, wxEXPAND | wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, wxID_ANY, _("Copyright: ")),
                    0, wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, wxID_ANY, APP_COPYRIGTH),
                    1, wxEXPAND | wxALIGN_LEFT);

    // about icontitle//info
    wxBoxSizer *aboutpane = new wxBoxSizer (wxHORIZONTAL);
    wxBitmap bitmap = wxBitmap(wxICON (sample));
    aboutpane->Add (new wxStaticBitmap (this, wxID_ANY, bitmap),
                    0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 20);
    aboutpane->Add (aboutinfo, 1, wxEXPAND);
    aboutpane->Add (60, 0);

    // about complete
    wxBoxSizer *totalpane = new wxBoxSizer (wxVERTICAL);
    totalpane->Add (0, 20);
    wxStaticText *appname = new wxStaticText(this, wxID_ANY, *g_appname);
    appname->SetFont (wxFont (24, wxDEFAULT, wxNORMAL, wxBOLD));
    totalpane->Add (appname, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT, 40);
    totalpane->Add (0, 10);
    totalpane->Add (aboutpane, 0, wxEXPAND | wxALL, 4);
    totalpane->Add (new wxStaticText(this, wxID_ANY, APP_DESCR),
                    0, wxALIGN_CENTER | wxALL, 10);
    wxButton *okButton = new wxButton (this, wxID_OK, _("OK"));
    okButton->SetDefault();
    totalpane->Add (okButton, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizerAndFit (totalpane);

    CenterOnScreen();
    ShowModal();
}

AppAbout::~AppAbout () {
    wxDELETE(m_timer);
}

//----------------------------------------------------------------------------
// event handlers
void AppAbout::OnTimerEvent (wxTimerEvent &WXUNUSED(event)) {
    wxDELETE(m_timer);
    EndModal (wxID_OK);
}
