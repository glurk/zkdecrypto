#ifndef __BASE_H 
// Make sure to only declare these classes once 
#define __BASE_H 
#include "z340.h"
#include "wx/thread.h"

class MainApp: public wxApp // MainApp is the class for our application 
{
	// MainApp just acts as a container for the window, or frame in MainFrame
public:
	virtual bool OnInit(); 
};
 
class MyWorkerThread;

std::string IntToString(int);

class MainFrame: public wxFrame // MainFrame is the class for our window, 
{ 
	// It contains the window and all objects in it 
public: 
	MainFrame( const wxString &title, const wxPoint &pos, const wxSize &size );
	wxTextCtrl *CipherText;
	wxTextCtrl *PlainText;
	wxTextCtrl *StartKey;
	wxTextCtrl *BestKey;
	wxTextCtrl *Score;
	wxButton *StartButton;
	wxMenuBar *MainMenu;
	wxStaticText *lblCipherText;
	wxStaticText *lblPlainText;
	wxStaticText *lblStartKey;
	wxStaticText *lblBestKey;
	wxStaticText *lblScore;
	void Quit(wxCommandEvent& event);
    void NewFile(wxCommandEvent& event);
    void OpenFile(wxCommandEvent& event);
    void SaveFile(wxCommandEvent& event);
    void SaveFileAs(wxCommandEvent& event);
    void CloseFile(wxCommandEvent& event);
	void StartButton_Click(wxCommandEvent& event);
	void StartCracking();
	void UpdatePlainText(wxCommandEvent& event);
	void UpdateBestKey(wxCommandEvent& event);
	void UpdateScore(wxCommandEvent& event);
	MyWorkerThread* mwt;

	DECLARE_EVENT_TABLE()
}; 

class MyWorkerThread : public wxThread
{
public:
    MyWorkerThread(MainFrame *frame);

    // thread execution starts here
    virtual void *Entry();

    // called when the thread exits - whether it terminates normally or is
    // stopped with Delete() (but not when it is Kill()ed!)
    virtual void OnExit();

public:
    MainFrame *m_frame;
    unsigned m_count;
	char ciphertext[MAX_CIPH_LENGTH];
	char startkey[ASCII_SIZE];
};
#endif