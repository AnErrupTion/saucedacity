/**********************************************************************

  Audacity: A Digital Audio Editor

  Export.h

  Dominic Mazzoni

**********************************************************************/

#ifndef __AUDACITY_EXPORT__
#define __AUDACITY_EXPORT__

#include <functional>
#include <vector>
#include <wx/filename.h> // member variable

// Saucedacity libraries
#include <lib-files/FileNames.h>
#include <lib-math/SampleFormat.h>
#include <lib-registries/Registry.h>
#include <lib-strings/Identifier.h>

#include "../widgets/wxPanelWrapper.h" // to inherit

class wxArrayString;
class FileDialogWrapper;
class wxFileCtrlEvent;
class wxMemoryDC;
class wxSimplebook;
class wxStaticText;
class SaucedacityProject;
class WaveTrack;
class Tags;
class TrackList;
class MixerSpec;
class ProgressDialog;
class ShuttleGui;
class Mixer;
using WaveTrackConstArray = std::vector < std::shared_ptr < const WaveTrack > >;
namespace GenericUI{ enum class ProgressResult : unsigned; }
class wxFileNameWrapper;

class SAUCEDACITY_DLL_API FormatInfo
{
   public:
      FormatInfo() {}
      FormatInfo( const FormatInfo & ) = default;
      FormatInfo &operator = ( const FormatInfo & ) = default;
      //FormatInfo( FormatInfo && ) = default;
      //FormatInfo &operator = ( FormatInfo && ) = default;
      ~FormatInfo() {}

      wxString mFormat;
      TranslatableString mDescription;
      // wxString mExtension;
      FileExtensions mExtensions;
      FileNames::FileTypes mMask;
      unsigned mMaxChannels;
      bool mCanMetaData;
};

//----------------------------------------------------------------------------
// ExportPlugin
//----------------------------------------------------------------------------
class SAUCEDACITY_DLL_API ExportPlugin /* not final */
{
public:
   using ProgressResult = GenericUI::ProgressResult;

   ExportPlugin();
   virtual ~ExportPlugin();

   int AddFormat();
   void SetFormat(const wxString & format, int index);
   void SetDescription(const TranslatableString & description, int index);
   void AddExtension(const FileExtension &extension, int index);
   void SetExtensions(FileExtensions extensions, int index);
   void SetMask(FileNames::FileTypes mask, int index);
   void SetMaxChannels(unsigned maxchannels, unsigned index);
   void SetCanMetaData(bool canmetadata, int index);

   virtual int GetFormatCount();
   virtual wxString GetFormat(int index);
   TranslatableString GetDescription(int index);
   /** @brief Return the (first) file name extension for the sub-format.
    * @param index The sub-format for which the extension is wanted */
   virtual FileExtension GetExtension(int index = 0);
   /** @brief Return all the file name extensions used for the sub-format.
    * @param index the sub-format for which the extension is required */
   virtual FileExtensions GetExtensions(int index = 0);
   FileNames::FileTypes GetMask(int index);
   virtual unsigned GetMaxChannels(int index);
   virtual bool GetCanMetaData(int index);

   virtual bool IsExtension(const FileExtension & ext, int index);

   virtual bool DisplayOptions(wxWindow *parent, int format = 0);
   
   virtual void OptionsCreate(ShuttleGui &S, int format) = 0;

   virtual bool CheckFileName(wxFileName &filename, int format = 0);
   /** @brief Exporter plug-ins may override this to specify the number
    * of channels in exported file. -1 for unspecified */
   virtual int SetNumExportChannels() { return -1; }

   /** \brief called to export audio into a file.
    *
    * @param pDialog To be initialized with pointer to a NEW ProgressDialog if
    * it was null, otherwise gives an existing dialog to be reused
   *  (working around a problem in wxWidgets for Mac; see bug 1600)
    * @param selectedOnly Set to true if all tracks should be mixed, to false
    * if only the selected tracks should be mixed and exported.
    * @param metadata A Tags object that will over-ride the one in *project and
    * be used to tag the file that is exported.
    * @param subformat Control which of the multiple formats this exporter is
    * capable of exporting should be used. Used where a single export plug-in
    * handles a number of related formats, but they have separate
    * entries in the Format drop-down list box. For example, the options to
    * export to "Other PCM", "AIFF 16 Bit" and "WAV 16 Bit" are all the same
    * libsndfile export plug-in, but with subformat set to 0, 1, and 2
    * respectively.
    * @return ProgressResult::Failed or ProgressResult::Cancelled if export
    * fails to complete for any reason, in which case this function is
    * responsible for alerting the user.  Otherwise ProgressResult::Success or
    * ProgressResult::Stopped
    */
   virtual ProgressResult Export(SaucedacityProject *project,
                       std::unique_ptr<ProgressDialog> &pDialog,
                       unsigned channels,
                       const wxFileNameWrapper &fName,
                       bool selectedOnly,
                       double t0,
                       double t1,
                       MixerSpec *mixerSpec = nullptr,
                       const Tags *metadata = nullptr,
                       int subformat = 0) = 0;

protected:
   static std::unique_ptr<Mixer> CreateMixer(const TrackList &tracks,
         bool selectionOnly,
         double startTime, double stopTime,
         unsigned numOutChannels, size_t outBufferSize, bool outInterleaved,
         double outRate, sampleFormat outFormat,
         MixerSpec *mixerSpec);

   // Create or recycle a dialog.
   static void InitProgress(std::unique_ptr<ProgressDialog> &pDialog,
         const TranslatableString &title, const TranslatableString &message);
   static void InitProgress(std::unique_ptr<ProgressDialog> &pDialog,
         const wxFileNameWrapper &title, const TranslatableString &message);

private:
   std::vector<FormatInfo> mFormatInfos;
};

using ExportPluginArray = std::vector < std::unique_ptr< ExportPlugin > > ;

//----------------------------------------------------------------------------
// Exporter
//----------------------------------------------------------------------------

// For a file suffix change from the options.
wxDECLARE_EXPORTED_EVENT(SAUCEDACITY_DLL_API,
   AUDACITY_FILE_SUFFIX_EVENT, wxCommandEvent);

class  SAUCEDACITY_DLL_API Exporter final : public wxEvtHandler
{
public:

   using ExportPluginFactory =
      std::function< std::unique_ptr< ExportPlugin >() >;

   // Objects of this type are statically constructed in files implementing
   // subclasses of ExportPlugin
   // Register factories, not plugin objects themselves, which allows them
   // to have some fresh state variables each time export begins again
   // and to compute translated strings for the current locale
   struct SAUCEDACITY_DLL_API RegisteredExportPlugin{
      RegisteredExportPlugin(
         const Identifier &id, // an internal string naming the plug-in
         const ExportPluginFactory&,
         const Registry::Placement &placement = { wxEmptyString, {} } );
   };

   static bool DoEditMetadata(SaucedacityProject &project,
      const TranslatableString &title,
      const TranslatableString &shortUndoDescription, bool force);

   Exporter( SaucedacityProject &project );
   ~Exporter() override;

   void SetFileDialogTitle( const TranslatableString & DialogTitle );
   void SetDefaultFormat( const FileExtension & Format ){ mFormatName = Format;};

   bool Process(bool selectedOnly,
                double t0, double t1);
   bool Process(unsigned numChannels,
                const FileExtension &type, const wxString & filename,
                bool selectedOnly, double t0, double t1);

   void DisplayOptions(int index);
   int FindFormatIndex(int exportindex);

   const ExportPluginArray &GetPlugins();

   // Auto Export from Timer Recording
   bool ProcessFromTimerRecording(bool selectedOnly,
                                  double t0,
                                  double t1,
                                  const wxFileName& fnFile,
                                  int iFormat,
                                  int iSubFormat,
                                  int iFilterIndex);
   bool SetAutoExportOptions();
   int GetAutoExportFormat() const;
   int GetAutoExportSubFormat() const;
   int GetAutoExportFilterIndex() const;
   wxFileName GetAutoExportFileName();
   void OnExtensionChanged(wxCommandEvent &evt);
   void OnHelp(wxCommandEvent &evt);

private:
   bool ExamineTracks();
   bool GetFilename();
   bool CheckFilename();
   bool CheckMix(bool prompt = true);
   bool ExportTracks();

   static void CreateUserPaneCallback(wxWindow *parent, wxUIntPtr userdata);
   void CreateUserPane(wxWindow *parent);
   void OnFilterChanged(wxFileCtrlEvent & evt);

private:
   FileExtension mFormatName;
   FileDialogWrapper *mDialog;
   TranslatableString mFileDialogTitle;
   SaucedacityProject *mProject;
   std::unique_ptr<MixerSpec> mMixerSpec;

   ExportPluginArray mPlugins;

   wxFileName mFilename;
   wxFileName mActualName;

   double mT0;
   double mT1;
   int mFilterIndex;
   int mFormat;
   int mSubFormat;
   int mNumSelected;
   unsigned mNumLeft;
   unsigned mNumRight;
   unsigned mNumMono;
   unsigned mChannels;
   bool mSelectedOnly;

   wxSimplebook *mBook;

   DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
// ExportMixerPanel
//----------------------------------------------------------------------------
class ExportMixerPanel final : public wxPanelWrapper
{
public:
   ExportMixerPanel( wxWindow *parent, wxWindowID id,
         MixerSpec *mixerSpec, const wxArrayString& trackNames,
         const wxPoint& pos = wxDefaultPosition,
         const wxSize& size = wxDefaultSize);
   ~ExportMixerPanel() override;

   void OnMouseEvent(wxMouseEvent & event);
   void OnPaint(wxPaintEvent & event);

private:
   std::unique_ptr<wxBitmap> mBitmap;
   wxRect mEnvRect;
   int mWidth;
   int mHeight;
   MixerSpec *mMixerSpec;
   ArrayOf<wxRect> mChannelRects;
   ArrayOf<wxRect> mTrackRects;
   int mSelectedTrack, mSelectedChannel;
   wxArrayString mTrackNames;
   int mBoxWidth, mChannelHeight, mTrackHeight;

   static void SetFont( wxMemoryDC &memDC, const wxString &text, int width, int height );
   static double Distance( wxPoint &a, wxPoint &b );
   bool IsOnLine( wxPoint p, wxPoint la, wxPoint lb );

   DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
// ExportMixerDialog
//----------------------------------------------------------------------------
class ExportMixerDialog final : public wxDialogWrapper
{
public:
   // constructors and destructors
   ExportMixerDialog( const TrackList * tracks, bool selectedOnly, unsigned maxNumChannels,
         wxWindow *parent, wxWindowID id, const TranslatableString &title,
         const wxPoint& pos = wxDefaultPosition,
         const wxSize& size = wxDefaultSize,
         long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );
   ~ExportMixerDialog() override;

   MixerSpec* GetMixerSpec() { return mMixerSpec.get(); }

private:
   wxStaticText *mChannelsText;
   std::unique_ptr<MixerSpec> mMixerSpec;
   wxArrayString mTrackNames;

private:
   void OnOk( wxCommandEvent &event );
   void OnCancel( wxCommandEvent &event );
   void OnMixerPanelHelp( wxCommandEvent &event );
   void OnSlider( wxCommandEvent &event );
   void OnSize( wxSizeEvent &event );

private:
   DECLARE_EVENT_TABLE()
};

SAUCEDACITY_DLL_API TranslatableString AudacityExportCaptionStr();
SAUCEDACITY_DLL_API TranslatableString AudacityExportMessageStr();

/// We have many Export errors that are essentially anonymous
/// and are distinguished only by an error code number.
/// Rather than repeat the code, we have it just once.
SAUCEDACITY_DLL_API void ShowExportErrorDialog(const wxString& ErrorCode,
   TranslatableString message = AudacityExportMessageStr(),
   const TranslatableString& caption = AudacityExportCaptionStr());

SAUCEDACITY_DLL_API
void ShowDiskFullExportErrorDialog(const wxFileNameWrapper &fileName);

#endif
