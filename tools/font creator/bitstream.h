#define ER	1
#define OK	0

typedef struct tagBinBuffer
{
	BYTE* data;
	BYTE byte;
	char end;
	short bit;
	ULONG seek;
	ULONG size;
} BIN_BUFFER;

void Buf_Bopen(BIN_BUFFER *buffer, BYTE *data, int size);
void Buf_Bclose(BIN_BUFFER* buffer);
int Buf_Bread_I(BYTE nb, BIN_BUFFER *buffer);
int Buf_Bread_M(BYTE nb, BIN_BUFFER *buffer);
int Buf_Btell(BIN_BUFFER *buffer);
void Buf_Bwrite_I(UINT val, BYTE nb, BIN_BUFFER *buffer);
void Buf_Bwrite_M(UINT val, BYTE nb, BIN_BUFFER *buffer);
void Buf_Bflush(BIN_BUFFER *buffer);
