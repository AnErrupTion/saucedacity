/**********************************************************************

  Saucedacity: A Digital Audio Editor

  SaucedacityApp.h

  Dominic Mazzoni

  This is the main source file for Audacity which handles
  initialization and termination by subclassing wxApp.

**********************************************************************/

#ifndef __AUDACITY_APP__
#define __AUDACITY_APP__


// Saucedacity libraries
#include <lib-strings/Identifier.h>

#include <wx/app.h> // to inherit
#include <wx/timer.h> // member variable

#include <memory>

#ifdef __UNIX__
#include <semaphore.h>
#endif

class wxSingleInstanceChecker;
class wxSocketEvent;
class wxSocketServer;

class IPCServ;
class Importer;
class CommandHandler;
class AppCommandEvent;
class SaucedacityProject;

class SaucedacityApp final : public wxApp {
 public:
   SaucedacityApp();
   ~SaucedacityApp() override;
   bool OnInit() override;
   bool InitPart2();
   int OnExit() override;
   void OnFatalException() override;
   bool OnExceptionInMainLoop() override;

   // These are currently only used on macOS, where it's
   // possible to have a menu bar but no windows open.  It doesn't
   // hurt any other platforms, though.
   static void OnMenuAbout(wxCommandEvent & event);
   static void OnMenuNew(wxCommandEvent & event);
   static void OnMenuOpen(wxCommandEvent & event);
   static void OnMenuPreferences(wxCommandEvent & event);
   void OnMenuExit(wxCommandEvent & event);

   void OnQueryEndSession(wxCloseEvent & event);
   void OnEndSession(wxCloseEvent & event);

   // Most Recently Used File support (for all platforms).
   void OnMRUClear(wxCommandEvent &event);
   void OnMRUFile(wxCommandEvent &event);
   // Backend for above - returns true for success, false for failure
   static bool MRUOpen(const FilePath &fileName);
   // A wrapper of the above that does not throw
   bool SafeMRUOpen(const wxString &fileName);

   void OnReceiveCommand(AppCommandEvent &event);

   void OnKeyDown(wxKeyEvent &event);

   void OnTimer(wxTimerEvent & event);

   // IPC communication
   void OnServerEvent(wxSocketEvent & evt);
   void OnSocketEvent(wxSocketEvent & evt);

   #ifdef __WXMAC__
    // In response to Apple Events
    void MacOpenFile(const wxString &fileName)  override;
    void MacPrintFile(const wxString &fileName)  override;
    void MacNewFile()  override;
   #endif

   #if defined(__WXMSW__) && !defined(__WXUNIVERSAL__) && !defined(__CYGWIN__)
    void AssociateFileTypes();
   #endif

#ifdef __WXMAC__

   void MacActivateApp();
   void MacFinishLaunching();

#endif


 private:
   std::unique_ptr<CommandHandler> mCmdHandler;

   std::unique_ptr<wxSingleInstanceChecker> mChecker;

   wxTimer mTimer;

   void InitCommandHandler();

   static bool InitTempDir();
   bool CreateSingleInstanceChecker(const wxString &dir);

   std::unique_ptr<wxCmdLineParser> ParseCommandLine();

#if defined(__WXMSW__)
   std::unique_ptr<IPCServ> mIPCServ;
#else
   std::unique_ptr<wxSocketServer> mIPCServ;

   sem_t* mLockSemaphore;

   static constexpr const char* LockSemName   = "/SaucedacityLock";
   static constexpr const char* SharedMemName = "/SaucedacityMem";
   bool mWasServer;

   /** @brief Cleans up ALL IPC resources created by any "server" processes.
    *
    * It is GUARANTEED that ALL RESOURCES created by the "server" process are
    * cleaned up and removed from the system at exit, unless something
    * catastrophic happens.
    *
    **/
   void CleanupIPCResources() const;
#endif

 public:
    DECLARE_EVENT_TABLE()
};

extern SaucedacityApp & wxGetApp();

#endif
