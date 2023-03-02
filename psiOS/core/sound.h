#ifndef SPU_H
#define SPU_H

enum VOICE_ALLOCATION
{
	VOICE_STREAM_L,					// spu streaming
	VOICE_STREAM_R,
	VOICE_SYSTEM,					// mostly menu related sounds
	VOICE_DYNAMIC=VOICE_SYSTEM+4	// dynamic voice allocation
};

#define NUM_SFX_VOICES			24
#define SPU_RAM					0x1010
#define RMS_MAX					16
#define SPU_MALLOC_MAX			RMS_MAX
#define SPU_BUFSIZE				(8*1024)	// size allocated for one streaming voice
#define SPU_BUFSIZEHALF			(SPU_BUFSIZE/2)
#define RAM_BUFSIZE				(16*1024)	// holds incoming data flow for streaming

#define VOICE_LIMIT				2

#define ST_VOICES				SPU_VOICECH(VOICE_STREAM_L)|SPU_VOICECH(VOICE_STREAM_R)

#define RMS_MAGIC				MAGIC('J','E','M','s')

enum SOUND_MODE
{
	SND_STEREO,
	SND_MONO
};

typedef struct tagRmsHeader
{
	int magic;					// JEMs
	int count;					// entries
	int size;					// size of VAG data
	int data;					// vag data
} RMS_HEADER;

typedef struct tagRmsEntry
{
	int freq;					// frequency
	int pos;					// pointer to data
	int size;					// in bytes
	int reserve;				// padding
} RMS_ENTRY;

// system
void SsInit();
void SsQuit();
void SsClearReverb();

// upload
int SsRmsLoad(char *filename);
int SsRmsFree(int id);
short SsRmsOpenHead(u_char *addr, short rms_id);
short SsRmsTransferBody(u_char *addr, short rms_id);

// playback
int SsRmsPlay(int rms_id, int sample, int voice, int voll, int volr);
int SsPlaySound(u_long addr, int voice, int pitch, int lvol, int rvol);

int SsFindOpenVoice();

#endif // SPU_H
