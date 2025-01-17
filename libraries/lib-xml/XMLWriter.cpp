/**********************************************************************

  Audacity: A Digital Audio Editor

  XMLWriter.cpp

  Leland Lucius

*******************************************************************//**

\class XMLWriter
\brief Base class for XMLFileWriter and XMLStringWriter that provides
the general functionality for creating XML in UTF8 encoding.

***********************************************************************/

#include "XMLWriter.h"

#include <wx/defs.h>
#include <wx/ffile.h>
#include <wx/intl.h>

#include <cstring>

//table for xml encoding compatibility with expat decoding
//see wxWidgets-2.8.12/src/expat/lib/xmltok_impl.h
//and wxWidgets-2.8.12/src/expat/lib/asciitab.h
static int charXMLCompatiblity[] =
  {

/* 0x00 */ 0, 0, 0, 0,
/* 0x04 */ 0, 0, 0, 0,
/* 0x08 */ 0, 1, 1, 0,
/* 0x0C */ 0, 1, 0, 0,
/* 0x10 */ 0, 0, 0, 0,
/* 0x14 */ 0, 0, 0, 0,
/* 0x18 */ 0, 0, 0, 0,
/* 0x1C */ 0, 0, 0, 0,
  };

// These are used by XMLEsc to handle surrogate pairs and filter invalid characters outside the ASCII range.
#define MIN_HIGH_SURROGATE static_cast<wxUChar>(0xD800)
#define MAX_HIGH_SURROGATE static_cast<wxUChar>(0xDBFF)
#define MIN_LOW_SURROGATE static_cast<wxUChar>(0xDC00)
#define MAX_LOW_SURROGATE static_cast<wxUChar>(0xDFFF)

// Unicode defines other noncharacters, but only these two are invalid in XML.
#define NONCHARACTER_FFFE static_cast<wxUChar>(0xFFFE)
#define NONCHARACTER_FFFF static_cast<wxUChar>(0xFFFF)


///
/// XMLWriter base class
///
XMLWriter::XMLWriter()
{
   mDepth = 0;
   mInTag = false;
   mHasKids.push_back(false);
}

XMLWriter::~XMLWriter()
{
}

void XMLWriter::StartTag(const wxString &name)
// may throw
{
   int i;

   if (mInTag) {
      Write(wxT(">\n"));
      mInTag = false;
   }

   for (i = 0; i < mDepth; i++) {
      Write(wxT("\t"));
   }

   Write(wxString::Format(wxT("<%s"), name));

   mTagstack.insert(mTagstack.begin(), name);
   mHasKids[0] = true;
   mHasKids.insert(mHasKids.begin(), false);
   mDepth++;
   mInTag = true;
}

void XMLWriter::EndTag(const wxString &name)
// may throw
{
   int i;

   if (!mTagstack.empty()) {
      if (mTagstack[0] == name) {
         if (mHasKids[1]) {  // There will always be at least 2 at this point
            if (mInTag) {
               Write(wxT("/>\n"));
            }
            else {
               for (i = 0; i < mDepth - 1; i++) {
                  Write(wxT("\t"));
               }
               Write(wxString::Format(wxT("</%s>\n"), name));
            }
         }
         else {
            Write(wxT(">\n"));
         }
         mTagstack.erase( mTagstack.begin() );
         mHasKids.erase(mHasKids.begin());
      }
   }

   mDepth--;
   mInTag = false;
}

void XMLWriter::WriteAttr(const wxString &name, const wxString &value)
// may throw from Write()
{
   Write(wxString::Format(wxT(" %s=\"%s\""),
      name,
      XMLEsc(value)));
}

void XMLWriter::WriteAttr(const wxString &name, const wxChar *value)
// may throw from Write()
{
   WriteAttr(name, wxString(value));
}

void XMLWriter::WriteAttr(const wxString &name, int value)
// may throw from Write()
{
   Write(wxString::Format(wxT(" %s=\"%d\""),
      name,
      value));
}

void XMLWriter::WriteAttr(const wxString &name, bool value)
// may throw from Write()
{
   Write(wxString::Format(wxT(" %s=\"%d\""),
      name,
      value));
}

void XMLWriter::WriteAttr(const wxString &name, long value)
// may throw from Write()
{
   Write(wxString::Format(wxT(" %s=\"%ld\""),
      name,
      value));
}

void XMLWriter::WriteAttr(const wxString &name, long long value)
// may throw from Write()
{
   Write(wxString::Format(wxT(" %s=\"%lld\""),
      name,
      value));
}

void XMLWriter::WriteAttr(const wxString &name, size_t value)
// may throw from Write()
{
   Write(wxString::Format(wxT(" %s=\"%lld\""),
      name,
      (long long) value));
}

void XMLWriter::WriteAttr(const wxString &name, float value, int digits)
// may throw from Write()
{
   Write(wxString::Format(wxT(" %s=\"%s\""),
      name,
      Internat::ToString(value, digits)));
}

void XMLWriter::WriteAttr(const wxString &name, double value, int digits)
// may throw from Write()
{
   Write(wxString::Format(wxT(" %s=\"%s\""),
      name,
      Internat::ToString(value, digits)));
}

void XMLWriter::WriteData(const wxString &value)
// may throw from Write()
{
   int i;

   for (i = 0; i < mDepth; i++) {
      Write(wxT("\t"));
   }

   Write(XMLEsc(value));
}

void XMLWriter::WriteSubTree(const wxString &value)
// may throw from Write()
{
   if (mInTag) {
      Write(wxT(">\n"));
      mInTag = false;
      mHasKids[0] = true;
   }

   Write(value);
}

// See http://www.w3.org/TR/REC-xml for reference
wxString XMLWriter::XMLEsc(const wxString & s)
{
   wxString result;
   int len = s.length();

   for(int i=0; i<len; i++) {
      wxUChar c = s.GetChar(i);

      switch (c) {
         case wxT('\''):
            result += wxT("&apos;");
         break;

         case wxT('"'):
            result += wxT("&quot;");
         break;

         case wxT('&'):
            result += wxT("&amp;");
         break;

         case wxT('<'):
            result += wxT("&lt;");
         break;

         case wxT('>'):
            result += wxT("&gt;");
         break;

         default:
            if (sizeof(c) == 2 && c >= MIN_HIGH_SURROGATE && c <= MAX_HIGH_SURROGATE && i < len - 1) {
               // If wxUChar is 2 bytes, then supplementary characters (those greater than U+FFFF) are represented
               // with a high surrogate (U+D800..U+DBFF) followed by a low surrogate (U+DC00..U+DFFF).
               // Handle those here.
               wxUChar c2 = s.GetChar(++i);
               if (c2 >= MIN_LOW_SURROGATE && c2 <= MAX_LOW_SURROGATE) {
                  // Surrogate pair found; simply add it to the output string.
                  result += c;
                  result += c2;
               }
               else {
                  // That high surrogate isn't paired, so ignore it.
                  i--;
               }
            }
            else if (!wxIsprint(c)) {
               //ignore several characters such ase eot (0x04) and stx (0x02) because it makes expat parser bail
               //see xmltok.c in expat checkCharRefNumber() to see how expat bails on these chars.
               //also see wxWidgets-2.8.12/src/expat/lib/asciitab.h to see which characters are nonxml compatible
               //post decode (we can still encode '&' and '<' with this table, but it prevents us from encoding eot)
               //everything is compatible past ascii 0x20 except for surrogates and the noncharacters U+FFFE and U+FFFF,
               //so we don't check the compatibility table higher than this.
               if((c> 0x1F || charXMLCompatiblity[c]!=0) &&
                     (c < MIN_HIGH_SURROGATE || c > MAX_LOW_SURROGATE) &&
                     c != NONCHARACTER_FFFE && c != NONCHARACTER_FFFF)
                  result += wxString::Format(wxT("&#x%04x;"), c);
            }
            else {
               result += c;
            }
         break;
      }
   }

   return result;
}
