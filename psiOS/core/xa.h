#define XA_SIZE		1

typedef struct XA_TRACK
{
	int start;
	int end;
} XA_TRACK;

void PrepareXA(void);
void UnprepareXA(int mode);

void PlayXA(int group, int index);
int StartXA(int startp, int endp, int index, int addon);
void PauseXA(void);
void StopXA(void);
void ResumeXA(void);

int PlayingXA(void);

