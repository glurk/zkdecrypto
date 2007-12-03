#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif 
#include "Zkdecrypto.h"
#include "wx/thread.h"
#include <string>
#include <sstream>


DEFINE_EVENT_TYPE(EVT_UpdatePlainText)
DEFINE_EVENT_TYPE(EVT_UpdateBestKey)
DEFINE_EVENT_TYPE(EVT_UpdateScore)


BEGIN_EVENT_TABLE ( MainFrame, wxFrame )
	EVT_MENU(MENU_New, MainFrame::NewFile)
	EVT_MENU(MENU_Open, MainFrame::OpenFile)
	EVT_MENU(MENU_Close, MainFrame::CloseFile)
	EVT_MENU(MENU_Save, MainFrame::SaveFile)
	EVT_MENU(MENU_SaveAs, MainFrame::SaveFileAs)
	EVT_MENU(MENU_Quit, MainFrame::Quit)
	EVT_BUTTON(Start_Button, MainFrame::StartButton_Click)
	EVT_COMMAND  (Plain_Text,EVT_UpdatePlainText,MainFrame::UpdatePlainText)
	EVT_COMMAND  (Best_Key,EVT_UpdateBestKey,MainFrame::UpdateBestKey)
	EVT_COMMAND  (Score_,EVT_UpdateScore,MainFrame::UpdateScore)
END_EVENT_TABLE()

IMPLEMENT_APP(MainApp) // A macro that tells wxWidgets to create an instance of our application 


bool MainApp::OnInit() 
{
	// Create an instance of our frame, or window 
	MainFrame *MainWin = new MainFrame(_("Zodiac Decrypto v1.0 By Michael Eaton and Brax Sisco"), wxPoint(1, 1), wxSize(640, 520));
	MainWin->Show(true); // show the window 
	SetTopWindow(MainWin); // and finally, set it as the main window 
	

	return true;
} 
 
MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size) 
: wxFrame((wxFrame *) NULL, -1, title, pos, size) 
{
    CreateStatusBar(2);
    MainMenu = new wxMenuBar();
    wxMenu *FileMenu = new wxMenu();
 
    FileMenu->Append(MENU_New, wxT("&New"),
      wxT("Create a new file"));
    FileMenu->Append(MENU_Open, wxT("&Open"),
      wxT("Open an existing file"));
    FileMenu->Append(MENU_Close, wxT("&Close"),
      wxT("Close the current document"));
    FileMenu->Append(MENU_Save, wxT("&Save"),
      wxT("Save the current document"));
    FileMenu->Append(MENU_SaveAs, wxT("Save &As"),
      wxT("Save the current document under a new file name"));
    FileMenu->Append(MENU_Quit, wxT("&Quit"),
      wxT("Quit the editor"));
 
    MainMenu->Append(FileMenu, wxT("File"));
    SetMenuBar(MainMenu);
	wxPanel *panel = new wxPanel(this, -1);

	CipherText = new wxTextCtrl(panel, Cipher_Text,
    wxEmptyString, wxPoint(15,30),wxSize(180,250),
    wxTE_MULTILINE , wxDefaultValidator, wxTextCtrlNameStr);

	CipherText->LoadFile("408.ascii.txt");

	PlainText = new wxTextCtrl(panel, Plain_Text,
    wxEmptyString, wxPoint(225,30),wxSize(180,250),
    wxTE_MULTILINE , wxDefaultValidator, wxTextCtrlNameStr);

	StartKey = new wxTextCtrl(panel, Start_Key,
    "AAAABCDDEEEEEEEEFFGHHHHIIIIJKLLLMNNNNOOOOOOPPRRRSSSSSTTTTTUUVWY", wxPoint(15,310),wxSize(450,20), 0, wxDefaultValidator, wxTextCtrlNameStr);
	
	BestKey = new wxTextCtrl(panel, Best_Key,
    wxEmptyString, wxPoint(15,360),wxSize(450,20), wxTE_READONLY, wxDefaultValidator, wxTextCtrlNameStr);

	Score = new wxTextCtrl(panel, Score_,
    wxEmptyString, wxPoint(15,410),wxSize(50,20), wxTE_READONLY, wxDefaultValidator, wxTextCtrlNameStr);

	StartButton = new wxButton(panel,Start_Button,
    "Start Cracking", wxPoint(500,310),wxSize(100,50), wxTE_READONLY, wxDefaultValidator, wxButtonNameStr);
	
	lblCipherText = new wxStaticText(panel, lblCipher_Text,
    "Cipher Text", wxPoint(15,5),wxSize(60,20),
    0 ,  wxStaticTextNameStr); 

	lblPlainText = new wxStaticText(panel, lblPlain_Text,
    "Plain Text", wxPoint(225,5),wxSize(60,20),
    0 ,  wxStaticTextNameStr); 

	lblStartKey = new wxStaticText(panel, lblStart_Key,
    "Start Key", wxPoint(15,290),wxSize(60,20),
    0 ,  wxStaticTextNameStr); 

	lblBestKey = new wxStaticText(panel, lblBest_Key,
    "Best Key", wxPoint(15,340),wxSize(60,20),
    0 ,  wxStaticTextNameStr); 

	lblScore = new wxStaticText(panel, lblScore_,
    "Score", wxPoint(15,390),wxSize(60,20),
    0 ,  wxStaticTextNameStr); 


}

  void MainFrame::NewFile(wxCommandEvent& WXUNUSED(event))
  {
  }
 
  void MainFrame::OpenFile(wxCommandEvent& WXUNUSED(event))
  {
    CipherText->LoadFile(wxT("base.h"));
  }
 
  void MainFrame::CloseFile(wxCommandEvent& WXUNUSED(event))
  {
    CipherText->Clear();
  }
 
  void MainFrame::SaveFile(wxCommandEvent& WXUNUSED(event))
  {
    CipherText->SaveFile(wxT("base.h"));
  }
 
  void MainFrame::SaveFileAs(wxCommandEvent& WXUNUSED(event))
  {
  }
 
  void MainFrame::Quit(wxCommandEvent& WXUNUSED(event))
  {
    Close(TRUE); // Tells the OS to quit running this process
  }

  void MainFrame::StartButton_Click(wxCommandEvent& WXUNUSED(event))
  {

     StartButton->Enable(false);

	 //char key[ASCII_SIZE];
	 //char ciphertext[MAX_CIPH_LENGTH];


	 mwt = new MyWorkerThread(this);

	 mwt->Create();
	 mwt->SetPriority(0);
	 //mwt->Create();
	 for(int i=0;i<MAX_CIPH_LENGTH;i++) mwt->ciphertext[i]=0;
	 for(int i=0;i<ASCII_SIZE;i++) mwt->startkey[i]=0;
	 for(int i = 0; i < CipherText->GetValue().Length(); i++)
	 {
		mwt->ciphertext[i] = CipherText->GetValue().c_str()[i];
	 }

	  for(int i = 0; i < StartKey->GetValue().Length(); i++)
	 {
		mwt->startkey[i] = StartKey->GetValue().c_str()[i];
	 }
	 mwt->Run();
  }
  void MainFrame::UpdatePlainText(wxCommandEvent& event)
  {
	PlainText->SetValue(event.GetString());
  }
    void MainFrame::UpdateBestKey(wxCommandEvent& event)
  {
	BestKey->SetValue(event.GetString());
  }

	void MainFrame::UpdateScore(wxCommandEvent& event)
  {
	Score->SetValue(IntToString(event.GetInt()));
  }


	MyWorkerThread::MyWorkerThread(MainFrame *frame)
			: wxThread()
	{
		m_frame = frame;
		m_count = 0;
	}

	void MyWorkerThread::OnExit()
	{
	}

	void *MyWorkerThread::Entry()
	{
		hillclimb(ciphertext,startkey,(unsigned)strlen(ciphertext),m_frame);
		return NULL;
	}

	std::string IntToString(int i)
	{
		std::ostringstream s;
		s<<i;
		//CString s;
		return s.str();
	}