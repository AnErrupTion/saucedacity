/**********************************************************************

  Saucedacity: A Digital Audio Editor

  AuStaticText.h

  Avery King split from Theme.h

  This file is licensed under the wxWidgets license, see License.txt

**********************************************************************/

#pragma once

#include <wx/window.h>
#include <wx/dcclient.h>

/// Like wxStaticText, except it allows for theming unlike wxStaticText
class SAUCEDACITY_DLL_API AuStaticText : public wxWindow
{
    public:
        AuStaticText(wxWindow* parent, const wxString& text);

        void OnPaint(wxPaintEvent & evt);
        [[nodiscard]] bool AcceptsFocus() const override { return false; }

        void OnErase(wxEraseEvent& event)
        {
            static_cast<void>(event);
        };

        DECLARE_EVENT_TABLE();
};
