/**********************************************************************

  Audacity: A Digital Audio Editor

  ImportMIDI.h

  Dominic Mazzoni

*******************************************************************//**

\class MIDIParser
\brief Unused class that might someday be used again to read MIDI files
into a NoteTrack.

*//*******************************************************************/


#ifndef _IMPORT_MIDI_
#define _IMPORT_MIDI_


#include "Identifier.h"

#if defined(USE_MIDI)

class SaucedacityProject;
class NoteTrack;

bool DoImportMIDI( SaucedacityProject &project, const FilePath &fileName );

bool ImportMIDI(const FilePath &fName, NoteTrack * dest);

class MIDIParser {
 public:
   MIDIParser();

   void Parse();

   unsigned char *buffer;
   int bufferLen;
   int index;

   NoteTrack *noteTrack;

   int midifile_error;

   int keys[16][128];
   int division;
   double tempo;

 private:

   int check_aborted();

   int Mf_getc();
   void Mf_error(char *);
   void Mf_on(int, int, int);
   void Mf_off(int, int, int);
   void Mf_header(int, int, int);
   void Mf_tempo(int);

   void Mf_starttrack() {}
   void Mf_endtrack() {}
   void Mf_eot() {}
   void Mf_pressure(int, int, int) {}
   void Mf_controller(int, int, int) {}
   void Mf_pitchbend(int, int, int) {}
   void Mf_program(int, int) {}
   void Mf_chanpressure(int, int) {}
   void Mf_sysex(int, char *) {}
   void Mf_arbitrary(int, char *) {}
   void Mf_metamisc(int, int, char *) {}
   void Mf_seqnum(int) {}
   void Mf_smpte(int, int, int, int, int) {}
   void Mf_timesig(int, int, int, int) {}
   void Mf_keysig(int, int) {}
   void Mf_sqspecific(int, char *) {}
   void Mf_text(int, int, char *) {}

   int Mf_nomerge;              /* 1 => continue'ed system exclusives are */
   /* not collapsed. */
   long Mf_currtime;            /* current time in delta-time units */
   int Mf_skipinit;

   long Mf_toberead;

   int abort_flag;

   long readvarinum();
   long read32bit();
   int read16bit();
   void msgenlarge();
   char *msg();
   int readheader();
   void readtrack();
   void sysex();
   void msginit();
   int egetc();
   int msgleng();

   int readmt(char *, int);
   long to32bit(int, int, int, int);
   int to16bit(int, int);
   void mferror(char *);
   void badbyte(int);
   void metaevent(int);
   void msgadd(int);
   void chanmessage(int, int, int);

   char *Msgbuff;               /* message buffer */
   int Msgsize;                 /* Size of currently allocated Msg */
   int Msgindex;                /* index of next available location in Msg */
};

#endif

#endif
