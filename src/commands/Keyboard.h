/**********************************************************************

  Audacity: A Digital Audio Editor

  Keyboard.h

  Dominic Mazzoni
  Brian Gunlogson

**********************************************************************/

#ifndef __AUDACITY_KEYBOARD__
#define __AUDACITY_KEYBOARD__

#include "Identifier.h"
#include <wx/defs.h>

class wxKeyEvent;

struct NormalizedKeyStringTag;
// Case insensitive comparisons
using NormalizedKeyStringBase = TaggedIdentifier<NormalizedKeyStringTag, false>;

struct SAUCEDACITY_DLL_API NormalizedKeyString : NormalizedKeyStringBase
{
   NormalizedKeyString() = default;
   explicit NormalizedKeyString( const wxString &key );

   [[nodiscard]] wxString Display(bool usesSpecialChars = false) const;
};

namespace std
{
   template<> struct hash< NormalizedKeyString >
      : hash< NormalizedKeyStringBase > {};
}

SAUCEDACITY_DLL_API
NormalizedKeyString KeyEventToKeyString(const wxKeyEvent & keyEvent);

#endif
