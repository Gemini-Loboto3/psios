u16* wcscat(u16 *dst, const u16 *src);
u16* wcsncat(u16* front, const u16* back, size_t count);
int wcscmp(const u16 *src, const u16 *dst);
int wcsncmp(const u16* first, const u16* last, size_t count);
u16* wcscpy(u16 * dst, const u16 * src);
u16* wcsncpy(u16* dest, const u16* source, size_t count);
size_t wcslen(const u16 *wcs);

u16* wcschr(const u16* string, u16 ch);
u16* wcsrchr(const u16* string, u16 ch);
u16* wcspbrk(const u16* string, const u16* control);
size_t wcsspn(const u16* string, const u16* control);
size_t wcscspn(const u16* string, const u16* control);
u16* wsctok(u16 *s1, const u16* delimit);
u16* wcsstr(const u16* wcs1, const u16* wcs2);

#define _T(x)	(u16*)L x
