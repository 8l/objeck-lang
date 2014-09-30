//////////////////////////////////////////////////////////////////////////////
// Original authors:  Wyo, John Labenski, Otto Wyss
// Copyright: (c) wxGuide, (c) John Labenski, Otto Wyss
// Modified by: Randy Hollines
// Licence: wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "ide.h"

#include <wx/sstream.h>
#include <wx/mstream.h>
#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/radiobut.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/statbox.h>
#include <wx/dialog.h>
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/platinfo.h>
#include <wx/tokenzr.h>

/////////////////////////
// MyApp
/////////////////////////

bool MyApp::OnInit() 
{
  if (!wxApp::OnInit()) {
    return false;
  }

  wxFrame* frame = new MyFrame(NULL, wxID_ANY, wxT("Objeckor"), wxDefaultPosition, wxSize(800, 600));
  frame->Show();

  return true;
}

DECLARE_APP(MyApp)
IMPLEMENT_APP(MyApp)

//----------------------------------------------------------------------------
//! MyFrame

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
// common
EVT_CLOSE(MyFrame::OnClose)
// file
EVT_MENU(wxID_NEW, MyFrame::OnFileNew)
EVT_MENU(myID_NEW_FILE, MyFrame::OnFileNew)
EVT_MENU(myID_OPEN_FILE, MyFrame::OnFileOpen)
EVT_MENU(wxID_SAVE, MyFrame::OnFileSave)
EVT_MENU(wxID_SAVEAS, MyFrame::OnFileSaveAs)
EVT_MENU(wxID_CLOSE, MyFrame::OnFileClose)
// project
EVT_MENU(myID_NEW_PROJECT, MyFrame::OnProjectNew)
EVT_MENU(myID_OPEN_PROJECT, MyFrame::OnProjectOpen)
EVT_MENU(myID_CLOSE_PROJECT, MyFrame::OnProjectClose)
EVT_MENU(myID_BUILD_PROJECT, MyFrame::OnProjectBuild)
EVT_MENU(myID_PROJECT_ADD_FILE, MyFrame::OnAddProjectFile)
EVT_MENU(myID_PROJECT_REMOVE_FILE, MyFrame::OnRemoveProjectFile)
// find/replace
EVT_MENU(myID_DLG_FIND_TEXT, MyFrame::OnEdit)
EVT_MENU(myID_FINDNEXT, MyFrame::OnEdit)
EVT_MENU(myID_DLG_REPLACE_TEXT, MyFrame::OnEdit)
EVT_MENU(myID_REPLACENEXT, MyFrame::OnEdit)
EVT_MENU(myID_DLG_OPTIONS, MyFrame::OnOptions)
// editor operations
EVT_MENU(wxID_UNDO, MyFrame::OnEdit)
EVT_MENU(wxID_REDO, MyFrame::OnEdit)
EVT_MENU(wxID_SELECTALL, MyFrame::OnEdit)
EVT_MENU_RANGE(wxID_EDIT, wxID_PROPERTIES, MyFrame::OnEdit)
EVT_MENU_RANGE(myID_EDIT_FIRST, myID_EDIT_LAST, MyFrame::OnEdit)
END_EVENT_TABLE()

MyFrame::MyFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : 
    wxFrame(parent, id, title, pos, size, style) 
{
  m_optionsManager = new GeneralOptionsManager(wxT("ide.ini"));
  m_projectManager = NULL;
  m_newPageCount = 1;
  
  // setup window manager
  m_auiManager.SetManagedWindow(this);
  m_auiManager.AddPane(CreateTreeCtrl(), wxAuiPaneInfo().Left().PaneBorder(false));
  m_notebook = CreateNotebook(), 
  m_auiManager.AddPane(m_notebook, wxAuiPaneInfo().CenterPane().PaneBorder(false));
  m_auiManager.AddPane(CreateInfoCtrl(), wxAuiPaneInfo().Bottom().PaneBorder(false));
  
  // set menu and status bars
  SetMenuBar(CreateMenuBar());
  CreateStatusBar();
  GetStatusBar()->SetStatusText(wxT("Ready"));

  // set windows sizes
  SetMinSize(wxSize(800, 600));
  
  // set tool bar
  m_auiManager.AddPane(DoCreateToolBar(), wxAuiPaneInfo().Name(wxT("toolbar")).Caption(wxT("Toolbar 3")).
                      ToolbarPane().Top().Row(1).Position(1));
  
  // update
  m_notebook->SetFocus();
  m_auiManager.Update();
}

MyFrame::~MyFrame() 
{
  if(m_optionsManager) {
    delete m_optionsManager;
    m_optionsManager = NULL;
  }

  if(m_projectManager) {
    delete m_projectManager;
    m_projectManager = NULL;
  }

  m_auiManager.UnInit();
}

void MyFrame::DoUpdate() 
{
  m_auiManager.Update();
}

// common event handlers
void MyFrame::OnEdit(wxCommandEvent &event) 
{
  if (m_notebook) {
    m_notebook->GetEventHandler()->ProcessEvent(event);
  }
}

void MyFrame::OnClose(wxCloseEvent &WXUNUSED(event))
{
  // close all pages and prompt to save files
  m_notebook->CloseAll();
  Destroy();
}

// file event handlers
void MyFrame::OnProjectOpen(wxCommandEvent &event)
{
   wxFileDialog fileDialog(this, L"Open Objeck Project", L"", L"", L"Project Files (*.obp)|*.obp", 
                           wxFD_OPEN|wxFD_FILE_MUST_EXIST);
   if(fileDialog.ShowModal() == wxID_OK) {
     m_projectManager = new ProjectManager(this, m_tree, fileDialog.GetPath());
   }
}

void MyFrame::OnAddProjectFile(wxCommandEvent &event)
{
  wxFileDialog fileDialog(this, L"Add Source File...", L"", L"", L"Source Files (*.obs)|*.obs", 
                          wxFD_OPEN|wxFD_FILE_MUST_EXIST);
  if(fileDialog.ShowModal() == wxID_OK) {
    AddProjectSource(fileDialog.GetPath());
  }
}

void MyFrame::OnRemoveProjectFile(wxCommandEvent &event)
{
  if(m_projectManager) {
    m_projectManager->RemoveFile(m_tree->GetData());
  }
}

void MyFrame::AddProjectSource(const wxString &full_path) 
{
  wxFileName source_file(full_path);
  const wxString file_name = source_file.GetFullName();
  if(m_projectManager) {
    m_projectManager->AddFile(file_name, full_path, true);
  }
}

void MyFrame::OnProjectClose(wxCommandEvent &event)
{
  if(m_projectManager) {
    delete m_projectManager;
    m_projectManager = NULL;
  }
}

void MyFrame::OnProjectBuild(wxCommandEvent &event)
{
  // TODO: prompt to save files before build...

  if(!m_projectManager) {
    wxMessageDialog fileOverWrite(this, wxT("There is no project loaded."), wxT("Build Project"));
    fileOverWrite.ShowModal();
    return;
  }

  wxString objeck_base_path = m_optionsManager->GetObjeckPath();
  if(objeck_base_path.Last() != wxFileName::GetPathSeparator()) {
    objeck_base_path += wxFileName::GetPathSeparator();
  }

  // build path to compiler exe
  wstring obr_exe = wxT("obc");
  wxPlatformInfo platform;
  if((platform.GetOperatingSystemId() & wxOS_WINDOWS) != 0) {
    obr_exe += wxT(".exe");
  }
  wxString obr_path_exe = wxT("bin");
  obr_path_exe += wxFileName::GetPathSeparator();
  obr_path_exe += obr_exe;

  const wxString obr_full_path = objeck_base_path + obr_path_exe;
  wxFileName objeck_compiler_file(obr_full_path);
  // ensure compiler exists
  if(!objeck_compiler_file.FileExists()) {
    wxMessageDialog fileOverWrite(this, wxT("Unable to find compiler executable, please check path."), wxT("Build Project"));
    fileOverWrite.ShowModal();
    return;
  }

  BuildProcess process; wxExecuteEnv env;
  env.env[wxT("OBJECK_LIB_PATH")] = objeck_base_path + wxT("bin");
  env.env[wxT("LANG")] = wxT("en_US.UTF-8");

  wxString cmd = wxT("\"");
  cmd += obr_full_path;
  cmd += wxT("\" -src ");

  wxArrayString source_files = m_projectManager->GetFiles();
  for(size_t i = 0; i < source_files.size(); ++i) {
    cmd += wxT("'");
    cmd += source_files[i];
    cmd += wxT("'");

    if(i + 1 < source_files.size()) {
      cmd += wxT(",");
    }
  }

  // TODO: add libs (from PM)


  // TODO: add dest (from PM)
  cmd += wxT(" -dest \"a.obe\"");

  int code = wxExecute(cmd.mb_str(), wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE, &process, &env);
  if(code < 0) {
    wxMessageDialog fileOverWrite(this, wxT("Unable to invoke compiler executable, please check file paths."), wxT("Build Project"));
    fileOverWrite.ShowModal();
    return;
  }

  wxString output = ReadInputStream(process.GetErrorStream());
  output += ReadInputStream(process.GetInputStream());

  switch(code) {
  case 0:
    m_buildOutput->BuildSuccess(output);
    GetStatusBar()->SetStatusText(wxT("Build successful"));
    break;

  case 2:
  case 3: {
    int error_num = m_buildOutput->ShowErrors(output);
    wxString build_status = wxString::Format(wxT("%d build error(s)"), error_num);
    GetStatusBar()->SetStatusText(build_status);
  }
    break;
  }
}

void MyFrame::OnProjectNew(wxCommandEvent &WXUNUSED(event))
{
  NewProject project_dialog(this);
  if(project_dialog.ShowModal() == wxID_OK) {
    const wxString name = project_dialog.GetName();
    const wxString path = project_dialog.GetPath();
    
    wxFileName full_name(path + wxFileName::GetPathSeparator() + name + wxT(".obp"));
    if(full_name.FileExists()) {
      wxMessageDialog fileOverWrite(this, wxT("File ") + full_name.GetFullName() + 
                                    wxT(" already exists.\nWould you like to overwrite it?"),
                                    "Overwrite File", wxCENTER | wxNO_DEFAULT | wxYES_NO | wxICON_INFORMATION);
      if(fileOverWrite.ShowModal() == wxID_YES) {
        m_projectManager = new ProjectManager(this, m_tree, name, full_name.GetFullPath());
      }
    }
    else {
      m_projectManager = new ProjectManager(this, m_tree, name, full_name.GetFullPath());
    }
  }
}

void MyFrame::OnFileNew(wxCommandEvent &WXUNUSED(event))
{
  m_notebook->Freeze();
  const wxString title = wxString::Format(wxT("new %zu"), m_newPageCount++);
  m_notebook->AddPage(new Edit(m_notebook), title);
  m_notebook->SetSelection(m_notebook->GetPageCount() - 1);
  m_notebook->Thaw();
}

void MyFrame::OnFileOpen(wxCommandEvent &WXUNUSED(event)) 
{
  wxFileDialog dlg(this, wxT("Open file"), wxEmptyString, wxEmptyString, 
    wxT("Objeck files (*.obs)|*.obs;*.obw|All types (*.*)|*.*"),
    wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
  if(dlg.ShowModal() != wxID_OK) {
    return;
  }
	const wxString path = dlg.GetPath();
  m_notebook->OpenFile(path);
}

void MyFrame::OnFileSave(wxCommandEvent &WXUNUSED(event)) 
{
  if (!m_notebook->GetEdit()) return;

  if (m_notebook->GetEdit()->Modified()) {
    m_notebook->GetEdit()->SaveFile();
  }
}

void MyFrame::OnFileSaveAs(wxCommandEvent &WXUNUSED(event)) 
{
  if (!m_notebook->GetEdit()) return;

  wxString filename = wxEmptyString;
  wxFileDialog dlg(this, wxT("Save file"), wxEmptyString, wxEmptyString, 
    wxT("Objeck files (*.obs)|*.obs;*.obw|All types (*.*)|*.*"), 
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if (dlg.ShowModal() != wxID_OK) return;
  filename = dlg.GetPath();
  m_notebook->GetEdit()->SaveFile(filename);
}

void MyFrame::OnFileClose(wxCommandEvent &WXUNUSED(event)) 
{
  if (!m_notebook->GetEdit()) return;

  // check to see if file has been modified
  if (m_notebook->GetEdit()->Modified()) {
    if (wxMessageBox(_("Text is not saved, save before closing?"), _("Close"),
      wxYES_NO | wxICON_QUESTION) == wxYES) {
      m_notebook->GetEdit()->SaveFile();
      if (m_notebook->GetEdit()->Modified()) {
        wxMessageBox(_("Text could not be saved!"), _("Close abort"),
          wxOK | wxICON_EXCLAMATION);
        return;
      }
    }
  }

  // close file
  m_notebook->GetEdit()->SetFilename(wxEmptyString);
  m_notebook->GetEdit()->ClearAll();
  m_notebook->GetEdit()->SetSavePoint();

  // destroy page
  const int page_index = m_notebook->GetSelection();
  if (page_index > -1) {
    m_notebook->DeletePage(page_index);
  }
}

void MyFrame::OnOptions(wxCommandEvent &WXUNUSED(event))
{
  m_optionsManager->ShowOptionsDialog(this);
}

wxMenuBar* MyFrame::CreateMenuBar()
{
  // File menu
  wxMenu* menuFile = new wxMenu;
  // new
  wxMenu* menuFileNew = new wxMenu;
  menuFileNew->Append(myID_NEW_FILE, _("&File\tCtrl+N"));
  menuFileNew->Append(myID_NEW_PROJECT, _("&Project\tCtrl+Shift+N"));
  menuFile->Append(wxID_ANY, _("New..."), menuFileNew);
  // open
  wxMenu* menuFileOpen = new wxMenu;
  menuFileOpen->Append(myID_OPEN_FILE, _("&File\tCtrl+O"));
  menuFileOpen->Append(myID_OPEN_PROJECT, _("&Project\tCtrl+Shift+O"));
  menuFile->Append(wxID_ANY, _("Open..."), menuFileOpen);
  
  menuFile->Append(wxID_SAVE, _("&Save\tCtrl+S"));
  menuFile->Append(wxID_SAVEAS, _("Save &as...\tCtrl+Shift+S"));

  // close
  wxMenu* menuFileClose = new wxMenu;
  menuFileClose->Append(myID_CLOSE_FILE, _("&File\tCtrl+W"));
  menuFileClose->Append(myID_CLOSE_PROJECT, _("&Project\tCtrl+Shift+W"));  
  menuFile->Append(wxID_ANY, _("Close..."), menuFileClose);
  
  menuFile->AppendSeparator();
  menuFile->Append(wxID_PROPERTIES, _("Proper&ties\tCtrl+T"));
  
  // edit menu
  wxMenu *menuEdit = new wxMenu;
  menuEdit->Append(wxID_UNDO, _("&Undo\tCtrl+Z"));
  menuEdit->Append(wxID_REDO, _("&Redo\tCtrl+Y"));
  menuEdit->AppendSeparator();
  menuEdit->Append(wxID_CUT, _("Cu&t\tCtrl+X"));
  menuEdit->Append(wxID_COPY, _("&Copy\tCtrl+C"));
  menuEdit->Append(wxID_PASTE, _("&Paste\tCtrl+V"));
  menuEdit->Append(wxID_SELECTALL, _("&Select All\tCtrl+A"));
  menuEdit->AppendSeparator();
  menuEdit->AppendCheckItem(myID_OVERTYPE, _("Over &type\tCtrl+Shift+T"));
  menuEdit->AppendCheckItem(myID_READONLY, _("Read-&only\tCtrl+Shift+R"));
  menuEdit->AppendCheckItem(myID_WRAPMODEON, _("&Word wrap\tCtrl+Shift+W"));
  menuEdit->AppendSeparator();
  menuEdit->Append(myID_DLG_FIND_TEXT, _("&Find...\tCtrl+F"));
  menuEdit->Append(myID_FINDNEXT, _("Find &next\tF3"));
  menuEdit->Append(myID_DLG_REPLACE_TEXT, _("&Replace...\tCtrl+H"));
  menuEdit->Append(myID_REPLACENEXT, _("&Replace &again\tShift+F3"));
  menuEdit->Append(myID_GOTO, _("&Go To...\tCtrl+G"));
  menuEdit->AppendSeparator();
  menuEdit->Append(myID_DLG_OPTIONS, _("General Options...\tALT+O"));
  
  // view menu
  wxMenu *menuView = new wxMenu;
  menuView->AppendCheckItem(myID_FOLDTOGGLE, _("&Toggle current fold\tCtrl+T"));
  menuView->AppendCheckItem(myID_OVERTYPE, _("&Overwrite mode\tIns"));
  menuView->AppendCheckItem(myID_WRAPMODEON, _("&Wrap mode\tCtrl+U"));
  menuView->AppendSeparator();
  menuView->AppendCheckItem(myID_DISPLAYEOL, _("Show line &endings"));
  menuView->AppendCheckItem(myID_INDENTGUIDE, _("Show &indent guides"));
  menuView->AppendCheckItem(myID_LINENUMBER, _("Show line &numbers"));
  menuView->AppendCheckItem(myID_LONGLINEON, _("Show &long line marker"));
  menuView->AppendCheckItem(myID_WHITESPACE, _("Show white&space"));

  // project menu
  m_projectView = new wxMenu;
  m_projectView->Append(myID_BUILD_PROJECT, _("Build\tCtrl+Shift+B"));
  m_projectView->AppendSeparator();
  m_projectView->Append(myID_ADD_FILE_PROJECT, _("&Add file...\tCtrl+Shift+A"));
  m_projectView->Append(myID_REMOVE_FILE_PROJECT, _("&Remove file...\tCtrl+Shift+R"));
  m_projectView->AppendSeparator();
  m_projectView->Append(myID_PROJECT_OPTIONS, _("Project options...\tALT+Shift+O"));
  DisableProjectMenu();

  // menu bar
  wxMenuBar* menuBar = new wxMenuBar;
  menuBar->Append(menuFile, wxT("&File"));
  menuBar->Append(menuEdit, wxT("&Edit"));
  menuBar->Append(m_projectView, wxT("&Project"));
  menuBar->Append(menuView, wxT("&View"));
  menuBar->Append(new wxMenu, wxT("&Help"));
  
  return menuBar;
}

wxAuiToolBar* MyFrame::DoCreateToolBar()
{
  wxAuiToolBarItemArray prepend_items;
  wxAuiToolBarItemArray append_items;
  wxAuiToolBar* toolbar = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
    wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW);
  toolbar->SetToolBitmapSize(wxSize(16, 16));
  
  wxBitmap tb3_bmp1 = wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16, 16));

  toolbar->AddTool(ID_SampleItem + 16, wxT("New File"), wxArtProvider::GetBitmap(wxART_NEW, wxART_OTHER, wxSize(16, 16)), wxT("Check 1"), wxITEM_CHECK);
  toolbar->AddTool(ID_SampleItem + 17, wxT("Open File"), wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_OTHER, wxSize(16, 16)), wxT("Check 2"), wxITEM_CHECK);
  toolbar->AddTool(ID_SampleItem + 18, wxT("Save File"), wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, wxSize(16, 16)), wxT("Check 3"), wxITEM_CHECK);
  toolbar->AddTool(ID_SampleItem + 19, wxT("Save File As..."), wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_OTHER, wxSize(16, 16)), wxT("Check 4"), wxITEM_CHECK);
  toolbar->AddSeparator();
  toolbar->AddTool(ID_SampleItem + 20, wxT("Copy"), wxArtProvider::GetBitmap(wxART_COPY, wxART_OTHER, wxSize(16, 16)), wxT("Copy"), wxITEM_RADIO);
  toolbar->AddTool(ID_SampleItem + 21, wxT("Cut"), wxArtProvider::GetBitmap(wxART_CUT, wxART_OTHER, wxSize(16, 16)), wxT("Cut"), wxITEM_RADIO);
  toolbar->AddTool(ID_SampleItem + 22, wxT("Paste"), wxArtProvider::GetBitmap(wxART_PASTE, wxART_OTHER, wxSize(16, 16)), wxT("Paste"), wxITEM_RADIO);
  toolbar->AddTool(ID_SampleItem + 23, wxT("Undo"), wxArtProvider::GetBitmap(wxART_UNDO, wxART_OTHER, wxSize(16, 16)), wxT("Undo"), wxITEM_RADIO);
  toolbar->AddTool(ID_SampleItem + 24, wxT("Redo"), wxArtProvider::GetBitmap(wxART_REDO, wxART_OTHER, wxSize(16, 16)), wxT("Redo"), wxITEM_RADIO);
  toolbar->AddSeparator();
  toolbar->AddTool(ID_SampleItem + 25, wxT("Build Project"), wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_OTHER, wxSize(16, 16)), wxT("Build Project"), wxITEM_RADIO);toolbar->AddSeparator();
  toolbar->AddTool(ID_SampleItem + 26, wxT("Project Options"), wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_OTHER, wxSize(16, 16)), wxT("Project Options"), wxITEM_RADIO);
  // toolbar->SetCustomOverflowItems(prepend_items, append_items);
  toolbar->Realize();

  return toolbar;
}

ProjectTreeCtrl* MyFrame::CreateTreeCtrl() 
{
  m_tree = new ProjectTreeCtrl(this, myID_PROJECT_TREE, wxPoint(0, 0), wxSize(160, 250), wxTR_DEFAULT_STYLE | wxNO_BORDER);

  wxImageList* imglist = new wxImageList(16, 16, true, 2);
  imglist->Add(wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_OTHER, wxSize(16, 16)));
  imglist->Add(wxArtProvider::GetBitmap(wxART_LIST_VIEW, wxART_OTHER, wxSize(16, 16)));
  imglist->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16)));
  imglist->Add(wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_OTHER, wxSize(16, 16)));
  m_tree->AssignImageList(imglist);

  m_tree->AddRoot(wxT("<empty>"), 0);
  
  return m_tree;
}

Notebook* MyFrame::CreateNotebook()
{
  // create the notebook off-window to avoid flicker
  wxSize client_size = GetClientSize();
  Notebook* notebook_ctrl = new Notebook(this, wxID_ANY, wxPoint(client_size.x, client_size.y), wxSize(430, 200), 
    wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

  notebook_ctrl->Freeze();
  const wxString title = wxT("new");
  notebook_ctrl->AddPage(new Edit(notebook_ctrl), title);
  notebook_ctrl->Thaw();

  return notebook_ctrl;
}

wxAuiNotebook* MyFrame::CreateInfoCtrl()
{
#ifdef __WXOSX_COCOA__
  wxFont font(11, wxMODERN, wxNORMAL, wxNORMAL);
#else
  wxFont font(9, wxMODERN, wxNORMAL, wxNORMAL);
#endif
  // build output
  m_buildOutput = new wxBuildErrorList(this, myID_BUILD_CTRL, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
  m_buildOutput->AppendColumn(wxT("#"));
  m_buildOutput->AppendColumn(wxT("File"));
  m_buildOutput->AppendColumn(wxT("Line"));
  m_buildOutput->AppendColumn(wxT("Message"));

  // runtime output
  m_executeOutput = new ExecuteTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTE_MULTILINE | wxTE_RICH);
  m_executeOutput->SetFont(font);
  
  wxAuiNotebook* info_ctrl = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(150, 100),
    wxAUI_NB_BOTTOM | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE);    
  info_ctrl->AddPage(m_buildOutput, wxT("Build"));
  info_ctrl->AddPage(m_executeOutput, wxT("Output"));

  return info_ctrl;
}

//----------------------------------------------------------------------------
//! BuildTextCtrl
wxBEGIN_EVENT_TABLE(wxBuildErrorList, wxListCtrl)
EVT_LIST_ITEM_ACTIVATED(myID_BUILD_CTRL, wxBuildErrorList::OnActivated)
wxEND_EVENT_TABLE()

wxBuildErrorList::wxBuildErrorList(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
  : wxListCtrl(parent, id, pos, size, style)
{

}

wxBuildErrorList::~wxBuildErrorList()
{

}

void wxBuildErrorList::OnActivated(wxListEvent& event)
{
  int row = event.GetIndex();
  if(row > -1) {
    wxString file = GetItemText(row, 1);
    wxString line = GetItemText(row, 2);

  }
}

void wxBuildErrorList::BuildSuccess(const wxString &output)
{
  DeleteAllItems();
}

int wxBuildErrorList::ShowErrors(const wxString &output)
{
  // parse line
  wxArrayString lines;
  wxStringTokenizer line_tokenizer(output, wxT("\r\n"));
  while(line_tokenizer.HasMoreTokens()) {
    lines.Add(line_tokenizer.GetNextToken());
  }
  
  // parse error line
  int error_count = 0;
  DeleteAllItems();
  for(size_t i = 0; i < lines.size(); ++i) {
    wxString line = lines[i];
    // additonal message output
    if(line.size() > 0 && line[0] == wxT('\t')) {
      // AppendText(line);
    }
    else {
      ++error_count;

      // parse error message
      wxArrayString error_parts;
      wxStringTokenizer message_tokenizer(line, wxT(":"));
      while(message_tokenizer.HasMoreTokens()) {
        error_parts.Add(message_tokenizer.GetNextToken());
      }

      // inspect parts
      size_t index = 0;
      
      // file name and path
      wxString full_path = error_parts[index++];
#ifdef __WXMSW__
      full_path += wxT(':');
      full_path += error_parts[index++];
#endif
      wxFileName source_file(full_path);

      // error id
      wxString error_id = wxString::Format(wxT("%zu"), i + 1);

      // line number
      const wxString line_nbr = error_parts[index++];

      // message
      wxString message = error_parts[index++];

      // set values
      int row = InsertItem(i, error_id);
      SetItem(row, 1, source_file.GetFullName());
      SetItem(row, 2, line_nbr);
      SetItem(row, 3, message);
    }
  }

  SetColumnWidth(0, wxLIST_AUTOSIZE);
  SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
  
  /*
  // m_buildOutput->SetColumnWidth(4, 0);

  int x = m_buildOutput->InsertItem(id1, wxT("1"));
  m_buildOutput->SetItem(x, 1, wxT("hello.obs"));
  m_buildOutput->SetItem(x, 2, wxT("3"));
  m_buildOutput->SetItem(x, 3, wxT("Expected ';' dkfkdasf kasdf adskf asdkf kasdfk asdkf"));

  m_buildOutput->SetColumnWidth(0, wxLIST_AUTOSIZE);
  m_buildOutput->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
  */

  return error_count;
}

//----------------------------------------------------------------------------
//! ExecuteTextCtrl
wxBEGIN_EVENT_TABLE(ExecuteTextCtrl, wxTextCtrl)

wxEND_EVENT_TABLE()

ExecuteTextCtrl::ExecuteTextCtrl(wxWindow* parent, wxWindowID id, const wxString &value, const wxPoint &pos, const wxSize &size, long style)
: wxTextCtrl(parent, id, value, pos, size, style)
{

}

ExecuteTextCtrl::~ExecuteTextCtrl()
{

}

//----------------------------------------------------------------------------
//! ProjectTreeCtrl
wxBEGIN_EVENT_TABLE(ProjectTreeCtrl, wxTreeCtrl)
EVT_TREE_ITEM_MENU(myID_PROJECT_TREE, ProjectTreeCtrl::OnItemMenu)
EVT_TREE_ITEM_ACTIVATED(myID_PROJECT_TREE, ProjectTreeCtrl::OnItemActivated)
wxEND_EVENT_TABLE()

ProjectTreeCtrl::ProjectTreeCtrl(MyFrame* parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
  : wxTreeCtrl(parent, id, pos, size, style)
{
  m_frame = parent;
}

void ProjectTreeCtrl::OnItemMenu(wxTreeEvent& event)
{
  wxTreeItemId itemId = event.GetItem();
  TreeData* data = (TreeData*)GetItemData(itemId);

  if(m_frame->GetProjectManager()) {
    if(m_frame->GetProjectManager()->HitProject(itemId)) {
      wxMenu menu;
      menu.Append(myID_BUILD_PROJECT, _("&Build"));
      menu.AppendSeparator();
      menu.Append(myID_PROJECT_ADD_FILE, _("&Add source..."));
      menu.Append(myID_PROJECT_ADD_LIB, _("&Add library..."));
      menu.AppendSeparator();
      menu.Append(wxID_ANY, wxT("&Project options"));
      PopupMenu(&menu, event.GetPoint());
    }
    else if(m_frame->GetProjectManager()->HitLibrary(itemId)) {
      wxMenu menu;
      menu.Append(myID_PROJECT_ADD_LIB, _("&Add library..."));
      PopupMenu(&menu, event.GetPoint());
    }
    else if(m_frame->GetProjectManager()->HitSource(itemId)) {
      wxMenu menu;
      menu.Append(myID_PROJECT_ADD_FILE, _("&Add source..."));
      PopupMenu(&menu, event.GetPoint());
    }
    else if(data) {
      wxMenu menu;
      menu.Append(myID_PROJECT_REMOVE_FILE, _("&Remove"));
      menu.AppendSeparator();
      menu.Append(wxID_ANY, wxT("&Properties"));

      item_data = data;
      PopupMenu(&menu, event.GetPoint());
    }
  }

  event.Skip();
}

void ProjectTreeCtrl::OnItemActivated(wxTreeEvent& event)
{
  wxTreeItemId itemId = event.GetItem();
  TreeData* item = (TreeData*)GetItemData(itemId);

  if(item) {
    const wxString full_path = item->GetFullPath();
    wxFileName source_file(full_path);
    if(source_file.Exists()) {
      m_frame->OpenFile(source_file.GetFullPath());
    }
    else {
      wxMessageDialog fileOverWrite(this, wxT("Unable to open file \'") + source_file.GetFullName() + 
                                    wxT("'.\nPlease check the full file path."), wxT("Unable to Open File"));
      fileOverWrite.ShowModal();
    }
  }
}



