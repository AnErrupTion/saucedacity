/**********************************************************************

   Audacity: A Digital Audio Editor
   Audacity(R) is copyright (c) 1999-2009 Audacity Team.
   File License: wxwidgets

   BatchEvalCommand.h
   Dan Horgan

******************************************************************//**

\class BatchEvalCommand
\brief Given a string representing a command, pass it to the MacroCommands
system.

The eventual aim is to move the code from MacroCommands out into separate
command classes, but until this happens, BatchEvalCommand can act as a 'bridge'
to that system.

*//*******************************************************************/

#ifndef __BATCHEVALCOMMAND__
#define __BATCHEVALCOMMAND__

#include "Command.h"
#include "CommandType.h"
#include "../BatchCommands.h"

class SaucedacityProject;

class BatchEvalCommandType final : public OldStyleCommandType
{
public:
   ComponentInterfaceSymbol BuildName() override;
   void BuildSignature(CommandSignature &signature) override;
   OldStyleCommandPointer Create( SaucedacityProject *project,
      std::unique_ptr<CommandOutputTargets> &&target) override;
};

class BatchEvalCommand final : public CommandImplementation
{
public:
   BatchEvalCommand(SaucedacityProject &project, OldStyleCommandType &type)
      : CommandImplementation(project, type)
   { }

   ~BatchEvalCommand() override;
   bool Apply(const CommandContext &context) override;
};

#endif /* End of include guard: __BATCHEVALCOMMAND__ */
