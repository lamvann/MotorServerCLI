#include <iostream>

using namespace std;

// for all others, include the necessary headers
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/socket.h"
#include "stdlib.h"
#include "string.h"

struct motor_struct {
    bool isStarted;
    int rotation;
    int speed;
} myMotor;

motor_struct testMotor;
//testMotor.isStarted = false;
//testMotor.rotation = 180;
//testMotor.speed = 0;


// *************** MyApp ************************************
class MyApp : public wxApp
// class MyApp : public wxAppConsole
{
public:
  MyApp();
  virtual bool OnInit();
  virtual int OnExit();
  //        virtual int OnRun();
  // event handlers (these functions should _not_ be virtual)
  void OnQuit(wxCommandEvent &event);
  void OnServerEvent(wxSocketEvent &event);
  void OnSocketEvent(wxSocketEvent &event);
  void OnServerStart();

private:
  wxSocketServer *m_server;

  // any class wishing to process wxWidgets events must use this macro
  DECLARE_EVENT_TABLE()
};

enum {
  // menu items
  SERVER_QUIT = 1000,
  SERVER_START,

  // id for sockets
  SERVER_ID,
  SOCKET_ID
};

// --------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// --------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MyApp, wxApp)
EVT_SOCKET(SERVER_ID, MyApp::OnServerEvent)
EVT_SOCKET(SOCKET_ID, MyApp::OnSocketEvent)
END_EVENT_TABLE()

void MyApp::OnServerStart() {
  // Create the address - defaults to localhost
  wxIPV4address addr;
  addr.Service(3000);

  // Create the socket, we maintain a class pointer so we can shut it down
  m_server = new wxSocketServer(addr);

  // We use Ok() here to see if the server is really listening
  if (!m_server->Ok()) {
    return;
  }

  wxPrintf(_("\nSocket server listening.\n"));

  // Setup the event handler and subscribe to connection events
  m_server->SetEventHandler(*this, SERVER_ID);
  m_server->SetNotify(wxSOCKET_CONNECTION_FLAG);
  m_server->Notify(true);
}

void MyApp::OnServerEvent(wxSocketEvent &WXUNUSED(event)) {
  // Accept the new connection and get the socket pointer
  wxSocketBase *sock = m_server->Accept(false);

  // Tell the new socket how and where to process its events
  sock->SetEventHandler(*this, SOCKET_ID);
  sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
  sock->Notify(true);

  wxPrintf(_("\nAccepted incoming connection.\n"));
}

void MyApp::OnSocketEvent(wxSocketEvent &event) {
  wxSocketBase *sock = event.GetSocket();

  // Process the event
  switch (event.GetSocketEvent()) {
  case wxSOCKET_INPUT: {
    char buf[20];
    char extraBuf[20];
    int leftDeg;
    int rightDeg;
    int spd;

    // Read the data
    sock->Read(buf, sizeof(buf));

    wxPrintf(wxString(_("Received from client: ")) +
             wxString(buf, wxConvUTF8, 10) + _("\n"));

    // Write it back
    sock->Write(buf, sizeof(buf));

    wxPrintf(_("Wrote string back to client.\n"));

    switch (buf[0]) {
    case '0':
      strcpy(buf, "OK. No command.\n");
      sock->Write(buf, sizeof(buf));
      break;
    case '1':
      strcpy(buf, "OK. Motor started.\n");
      sock->Write(buf, sizeof(buf));
      break;
    case '2':
      strcpy(buf, "OK. Motor stopped.\n");
      sock->Write(buf, sizeof(buf));
      break;
    case '3':
      snprintf(extraBuf, 20, "%s", buf + 1);
      leftDeg = atoi(extraBuf);
      strcpy(buf, "OK. Rotated left.\n\0");
      sock->Write(buf, sizeof(buf));
      break;
    case '4':
      snprintf(extraBuf, 20, "%s", buf + 1);
      rightDeg = atoi(extraBuf);
      strcpy(buf, "OK. Rotated right\n\0");
      sock->Write(buf, sizeof(buf));
      break;
    case '5':
      snprintf(extraBuf, 20, "%s", buf + 1);
      spd = atoi(extraBuf);
      strcpy(buf, "OK. Speed changed\n\0");
      sock->Write(buf, sizeof(buf));
      break;
    default:
      strcpy(buf, "Something went wrong\n\0");
      sock->Write(buf, sizeof(buf));
      break;
    }

    // We are done with the socket, destroy it
    sock->Destroy();

    break;
  }
  case wxSOCKET_LOST: {
    sock->Destroy();
    break;
  }
  }
}

MyApp::MyApp()
    : wxApp()
// MyApp::MyApp():wxAppConsole()
{}

bool MyApp::OnInit() {

  wxPrintf(_("Started Server.\n"));

  OnServerStart();

  return true; // on continue le traitement
}

int MyApp::OnExit() {
  cout << "dans OnExit" << endl;
  return 0;
}

IMPLEMENT_APP(MyApp);
