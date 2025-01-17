/**********************************************************************

  Audacity: A Digital Audio Editor

  MixerBoard.h

  Vaughan Johnson, January 2007

**********************************************************************/

#ifndef __AUDACITY_MIXER_BOARD__
#define __AUDACITY_MIXER_BOARD__

#include <wx/frame.h> // to inherit
#include <wx/scrolwin.h> // to inherit

#include "widgets/ASlider.h" // to inherit
#include "commands/CommandManagerWindowClasses.h"

// Saucedacity library
#include <lib-preferences/Prefs.h>

class wxArrayString;
class wxBitmapButton;
class wxImage;
class wxMemoryDC;
class AButton;
struct TrackListEvent;

// containment hierarchy:
//    MixerBoardFrame -> MixerBoard -> MixerBoardScrolledWindow -> MixerTrackCluster(s)


// MixerTrackSlider is a subclass just to override OnMouseEvent,
// so we can know when adjustment ends, so we can PushState only then.
class MixerTrackSlider final : public ASlider
{
public:
   MixerTrackSlider(wxWindow * parent,
                     wxWindowID id,
                     const TranslatableString &name,
                     const wxPoint & pos,
                     const wxSize & size,
                     const ASlider::Options &options = ASlider::Options{});
   ~MixerTrackSlider() override {}

   void OnMouseEvent(wxMouseEvent & event);

   void OnFocus(wxFocusEvent &event);
   void OnCaptureKey(wxCommandEvent& event);

protected:
   bool mIsPan;

public:
    DECLARE_EVENT_TABLE()
};


class SaucedacityProject;
class MeterPanel;
class MixerBoard;

class Track;
#ifdef USE_MIDI
class NoteTrack;
#endif
class PlayableTrack;

class WaveTrack;
class AuStaticText;

class MixerTrackCluster final : public wxPanelWrapper
{
public:
   MixerTrackCluster(wxWindow* parent,
                     MixerBoard* grandParent, SaucedacityProject* project,
                     const std::shared_ptr<PlayableTrack> &pTrack,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize);
   ~MixerTrackCluster() override {}

   [[nodiscard]] WaveTrack *GetWave() const;
   [[nodiscard]] WaveTrack *GetRight() const;
#ifdef EXPERIMENTAL_MIDI_OUT
   NoteTrack *GetNote() const;
#endif

   //void UpdatePrefs();

   void HandleResize(); // For wxSizeEvents, update gain slider and meter.

   void HandleSliderGain(bool bWantPushState = false);
#ifdef EXPERIMENTAL_MIDI_OUT
   void HandleSliderVelocity(const bool bWantPushState = false);
#endif
   void HandleSliderPan(bool bWantPushState = false);

   void ResetMeter(bool bResetClipping);

   void UpdateForStateChange();
   void UpdateMeter(double t0, double t1);

private:
   static wxColour GetTrackColor();

   // event handlers
   void HandleSelect(bool bShiftDown, bool bControlDown);

   void OnKeyEvent(wxKeyEvent& event);
   void OnMouseEvent(wxMouseEvent& event);
   void OnPaint(wxPaintEvent& evt);

   void OnButton_MusicalInstrument(wxCommandEvent& event);
   void OnSlider_Gain(wxCommandEvent& event);
#ifdef EXPERIMENTAL_MIDI_OUT
   void OnSlider_Velocity(wxCommandEvent& event);
#endif
   void OnSlider_Pan(wxCommandEvent& event);
   void OnButton_Mute(wxCommandEvent& event);
   void OnButton_Solo(wxCommandEvent& event);
   //v void OnSliderScroll_Gain(wxScrollEvent& event);


public:
   std::shared_ptr<PlayableTrack>   mTrack;

private:
   MixerBoard* mMixerBoard;
   SaucedacityProject* mProject;

   // controls
   AuStaticText* mStaticText_TrackName;
   wxBitmapButton* mBitmapButton_MusicalInstrument;
   AButton* mToggleButton_Mute;
   AButton* mToggleButton_Solo;
   MixerTrackSlider* mSlider_Pan;
   MixerTrackSlider* mSlider_Gain;
#ifdef EXPERIMENTAL_MIDI_OUT
   MixerTrackSlider* mSlider_Velocity;
#endif
   wxWeakRef<MeterPanel> mMeter;

public:
   DECLARE_EVENT_TABLE()
};


class MusicalInstrument
{
public:
   MusicalInstrument(std::unique_ptr<wxBitmap> &&pBitmap, const wxString & strXPMfilename);
   virtual ~MusicalInstrument();

   std::unique_ptr<wxBitmap> mBitmap;
   wxArrayString  mKeywords;
};

using MusicalInstrumentArray = std::vector<std::unique_ptr<MusicalInstrument>>;



// wxScrolledWindow ignores mouse clicks in client area,
// but they don't get passed to Mixerboard.
// We need to catch them to deselect all track clusters.
class MixerBoardScrolledWindow final : public wxScrolledWindow
{
public:
   MixerBoardScrolledWindow(SaucedacityProject* project,
                              MixerBoard* parent, wxWindowID id = -1,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxHSCROLL | wxVSCROLL);
   ~MixerBoardScrolledWindow() override;

private:
   void OnMouseEvent(wxMouseEvent& event);

private:
   MixerBoard* mMixerBoard;
   SaucedacityProject* mProject;

public:
   DECLARE_EVENT_TABLE()
};


class MixerBoardFrame;
class TrackList;

class MixerBoard final : public wxWindow, private PrefsListener
{
   friend class MixerBoardFrame;

public:
   MixerBoard(SaucedacityProject* pProject,
               wxFrame* parent,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize);

   void UpdatePrefs() override;

   // Add clusters for any tracks we're not yet showing.
   // Update pointers for tracks we're already showing.
   void UpdateTrackClusters();

   int GetTrackClustersWidth();


   wxBitmap* GetMusicalInstrumentBitmap(const Track *pTrack);

   bool HasSolo();

   void RefreshTrackClusters(bool bEraseBackground = true);
   void ResizeTrackClusters();

   void UpdateMeters(double t1, bool bLoopedPlay);

   void UpdateWidth();

private:
   void ResetMeters(bool bResetClipping);
   void RemoveTrackCluster(size_t nIndex);
   void MakeButtonBitmap( wxMemoryDC & dc, wxBitmap & bitmap,
      wxRect & bev, const TranslatableString & str, bool up );
   void CreateMuteSoloImages();
   int FindMixerTrackCluster(const PlayableTrack* pTrack,
                              MixerTrackCluster** hMixerTrackCluster) const;
   void LoadMusicalInstruments();

   // event handlers
   void OnPaint(wxPaintEvent& evt);
   void OnSize(wxSizeEvent &evt);
   void OnTimer(wxCommandEvent &event);
   void OnTrackSetChanged(wxEvent &event);
   void OnTrackChanged(TrackListEvent &event);
   void OnStartStop(wxCommandEvent &event);

public:
   // mute & solo button images: Create once and store on MixerBoard for use in all MixerTrackClusters.
   std::unique_ptr<wxImage> mImageMuteUp, mImageMuteOver, mImageMuteDown,
      mImageMuteDownWhileSolo, // the one actually alternate image
      mImageMuteDisabled, mImageSoloUp, mImageSoloOver, mImageSoloDown, mImageSoloDisabled;

   int mMuteSoloWidth;

private:
   // Track clusters are maintained in the same order as the WaveTracks.
   std::vector<MixerTrackCluster*> mMixerTrackClusters;

   MusicalInstrumentArray     mMusicalInstruments;
   SaucedacityProject*           mProject;
   MixerBoardScrolledWindow*  mScrolledWindow; // Holds the MixerTrackClusters and handles scrolling.
   double                     mPrevT1;
   TrackList*                 mTracks;
   bool                       mUpToDate{ false };

public:
   DECLARE_EVENT_TABLE()
};


class MixerBoardFrame final
   : public wxFrame
   , public TopLevelKeystrokeHandlingWindow
{
public:
   MixerBoardFrame(SaucedacityProject* parent);
   ~MixerBoardFrame() override;

   void Recreate(SaucedacityProject *pProject);

private:
   // event handlers
   void OnCloseWindow(wxCloseEvent &WXUNUSED(event));
   void OnMaximize(wxMaximizeEvent &event);
   void OnSize(wxSizeEvent &evt);
   void OnKeyEvent(wxKeyEvent &evt);

   void SetWindowTitle();

   SaucedacityProject *mProject;
public:
   MixerBoard* mMixerBoard;

public:
   DECLARE_EVENT_TABLE()
};

#endif // __AUDACITY_MIXER_BOARD__


