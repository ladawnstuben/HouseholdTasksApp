// Group 1 Final Project
// ITCS 2550
// Philip Seros, Wayne Williams, LaDawn Stuben
// 04/20/2024
// Household Task Manager
// This program allows users to create, manage, and track household tasks
// Using wxWidgets for GUI

#include <wx/wx.h> // Needed for wxWidgets
#include <wx/sizer.h> // Needed for wxSizer
#include <wx/stattext.h> // Needed for wxStaticText
#include <wx/button.h> // Needed for wxButton
#include <wx/panel.h> // Needed for wxPanel
#include <wx/menu.h> // Needed for wxMenu
#include <wx/msgdlg.h> // Needed for wxMessageBox
#include "wx/setup.h"



/////////////////////////////////////////////////////////////
// Test code to see if wxWidgets is functioning as expected//
////////////////////////////////////////////////////////////

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);

private:
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
};

wxIMPLEMENT_APP(MyApp);

MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(500, 400)) {

    // Menu Bar setup
    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(wxID_EXIT, "&Exit");
    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, "&About");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");
    SetMenuBar(menuBar);

    // Status Bar setup
    CreateStatusBar();
    SetStatusText("Welcome to Household Task Manager!");

    // Panel setup
    wxPanel* panel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Add static text
    wxStaticText* label = new wxStaticText(panel, wxID_ANY, "Welcome to your Household Task Manager!",
        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    sizer->Add(label, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 15);

    // Add a button
    wxButton* button = new wxButton(panel, wxID_ANY, "Start Managing Tasks!");
    sizer->Add(button, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

    panel->SetSizer(sizer);


    // Bind events
    Bind(wxEVT_COMMAND_MENU_SELECTED, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &MyFrame::OnAbout, this, wxID_ABOUT);
}

void MyFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("This is a simple wxWidgets' Hello World sample",
        "About Household Task Manager", wxOK | wxICON_INFORMATION);
}

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame("Household Task Manager");
    frame->Show(true);
    return true;
}
