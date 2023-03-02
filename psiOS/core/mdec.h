typedef struct tagSubData
{
	u_short ptr:15;
	u_short extra:1;		// if ending==1
	u_short start, end;
} SUB_DATA;

typedef struct tagSubChar
{
	u_short pos;
	u_char width;
	u_char heigth;
} SUB_CHAR;
