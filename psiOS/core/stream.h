#ifndef __STREAM_H
#define __STREAM_H

enum STREAM_OPS
{
	STREAM_START,
	STREAM_PLAY,
	STREAM_PAUSE,
	STREAM_STOP,
	STREAM_READING				// reading 32kb from disk
};

// streaming
int SsSetStream(int index);
void SsStreamSetPitch(int value);
void SsStreamSetReverb(int value);
void SsStreamPause();
void SsStreamResume();
void SsStreamStop();

#endif	// __STREAM_H
