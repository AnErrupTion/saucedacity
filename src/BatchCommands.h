/**********************************************************************

  Audacity: A Digital Audio Editor

  MacroCommands.h

  Dominic Mazzoni
  James Crook

**********************************************************************/

#ifndef __AUDACITY_BATCH_COMMANDS_DIALOG__
#define __AUDACITY_BATCH_COMMANDS_DIALOG__

#include <wx/defs.h>

#include "export/Export.h"
#include "commands/CommandFlag.h"

// Ssaucedaity libraries
#include <lib-components/ComponentInterface.h> // for ComponentInterfaceSymbol

class wxArrayString;
class Effect;
class CommandContext;
class CommandManager;
class SaucedacityProject;
class wxArrayStringEx;

class MacroCommandsCatalog {
public:
   // A triple of user-visible name, internal string identifier and type/help string.
   struct Entry {
      ComponentInterfaceSymbol name;
      TranslatableString category;
   };
   using Entries = std::vector<Entry>;

   explicit MacroCommandsCatalog( const SaucedacityProject *project );

   // binary search
   [[nodiscard]] Entries::const_iterator ByFriendlyName( const TranslatableString &friendlyName ) const;
   // linear search
   [[nodiscard]] Entries::const_iterator ByCommandId( const CommandID &commandId ) const;

   // Lookup by position as sorted by friendly name
   const Entry &operator[] ( size_t index ) const { return mCommands[index]; }

   [[nodiscard]] Entries::const_iterator begin() const { return mCommands.begin(); }
   [[nodiscard]] Entries::const_iterator end() const { return mCommands.end(); }

private:
   // Sorted by friendly name
   Entries mCommands;
};

// Stores information for one macro
class MacroCommands final {
 public:
   static bool DoAudacityCommand(
      const PluginID & ID, const CommandContext & context, unsigned flags );

   // constructors and destructors
   explicit MacroCommands( SaucedacityProject &project );
 public:
   bool ApplyMacro( const MacroCommandsCatalog &catalog,
      const wxString & filename = {});
   static bool HandleTextualCommand( CommandManager &commandManager,
      const CommandID & Str,
      const CommandContext & context, CommandFlag flags, bool alwaysEnabled);
   bool ApplyCommand( const TranslatableString &friendlyCommand,
      const CommandID & command, const wxString & params,
      CommandContext const * pContext=nullptr );
   bool ApplyCommandInBatchMode( const TranslatableString &friendlyCommand,
      const CommandID & command, const wxString &params,
      CommandContext const * pContext = nullptr);
   bool ApplyEffectCommand(
      const PluginID & ID, const TranslatableString &friendlyCommand,
      const CommandID & command,
      const wxString & params, const CommandContext & Context);
   static bool ReportAndSkip( const TranslatableString & friendlyCommand, const wxString & params );
   void AbortBatch();

   // These commands do not depend on the command list.
   static void MigrateLegacyChains();
   static wxArrayString GetNames();
   static wxArrayStringEx GetNamesOfDefaultMacros();

   static wxString GetCurrentParamsFor(const CommandID & command);
   static wxString PromptForParamsFor(
      const CommandID & command, const wxString & params, wxWindow &parent);
   static wxString PromptForPresetFor(const CommandID & command, const wxString & params, wxWindow *parent);

   // These commands do depend on the command list.
   void ResetMacro();

   void RestoreMacro(const wxString & name);
   wxString ReadMacro(const wxString & macro, wxWindow *parent = nullptr);
   wxString WriteMacro(const wxString & macro, wxWindow *parent = nullptr);
   static bool AddMacro(const wxString & macro);
   static bool DeleteMacro(const wxString & name);
   static bool RenameMacro(const wxString & oldmacro, const wxString & newmacro);

   void AddToMacro(const CommandID & command, int before = -1);
   void AddToMacro(const CommandID & command, const wxString & params, int before = -1);

   void DeleteFromMacro(int index);
   CommandID GetCommand(int index);
   wxString GetParams(int index);
   int GetCount();
   wxString GetMessage(){ return mMessage;};
   void AddToMessage(const wxString & msgIn ){ mMessage += msgIn;};

   static bool IsFixed(const wxString & name);

   static void Split(const wxString & str, wxString & command, wxString & param);
   static wxString Join(const wxString & command, const wxString & param);

private:
   SaucedacityProject &mProject;

   CommandIDs mCommandMacro;
   wxArrayString mParamsMacro;
   bool mAbort;
   wxString mMessage;

   Exporter mExporter;
   wxString mFileName;
};

#endif
