/*
 * A collection of libc string funcions made to use wchar_t instead of char.
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 *
 * http://archive.msdn.microsoft.com/vcsamplescrt/Project/License.aspx
 *
 * MICROSOFT PUBLIC LICENSE (Ms-PL)
 *
 * This license governs use of the accompanying software. If you use the software,
 * you accept this license. If you do not accept the license, do not use the software.
 *
 * 1. Definitions
 *	The terms "reproduce," "reproduction," "derivative works," and "distribution" have
 *	the same meaning here as under U.S. copyright law.
 *
 *	A "contribution" is the original software, or any additions or changes to the software.
 *
 *	A "contributor" is any person that distributes its contribution under this license.
 *
 *	"Licensed patents" are a contributor's patent claims that read directly on its contribution.
 * 
 * 2. Grant of Rights
 *
 *	(A) Copyright Grant- Subject to the terms of this license, including the license conditions
 *	and limitations in section 3, each contributor grants you a non-exclusive, worldwide,
 *	royalty-free copyright license to reproduce its contribution, prepare derivative works of
 *	its contribution, and distribute its contribution or any derivative works that you create.
 *
 *	(B) Patent Grant- Subject to the terms of this license, including the license conditions and
 *	limitations in section 3, each contributor grants you a non-exclusive, worldwide, royalty-free
 *	license under its licensed patents to make, have made, use, sell, offer for sale, import,
 *	and/or otherwise dispose of its contribution in the software or derivative works of the
 *	contribution in the software.
 *
 * 3. Conditions and Limitations
 *
 *	(A) No Trademark License- This license does not grant you rights to use any contributors' name,
 *	logo, or trademarks.
 *
 *	(B) If you bring a patent claim against any contributor over patents that you claim are infringed
 *	by the software, your patent license from such contributor to the software ends automatically.
 *
 *	(C) If you distribute any portion of the software, you must retain all copyright, patent,
 *	trademark, and attribution notices that are present in the software.
 *
 *	(D) If you distribute any portion of the software in source code form, you may do so only under
 *	this license by including a complete copy of this license with your distribution. If you distribute
 *	any portion of the software in compiled or object code form, you may only do so under a license
 *	that complies with this license.
 *
 *	(E) The software is licensed "as-is." You bear the risk of using it. The contributors give no
 *	express warranties, guarantees or conditions. You may have additional consumer rights under your
 *	local laws which this license cannot change. To the extent permitted under your local laws, the
 *	contributors exclude the implied warranties of merchantability, fitness for a particular purpose
 *	and non-infringement. 
 *
 *
 * CREATIVE COMMONS ATTRIBUTION 3.0 LICENSE
 * THE WORK (AS DEFINED BELOW) IS PROVIDED UNDER THE TERMS OF THIS CREATIVE COMMONS PUBLIC LICENSE
 * ("CCPL" OR "LICENSE"). THE WORK IS PROTECTED BY COPYRIGHT AND/OR OTHER APPLICABLE LAW. ANY USE OF
 * THE WORK OTHER THAN AS AUTHORIZED UNDER THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * BY EXERCISING ANY RIGHTS TO THE WORK PROVIDED HERE, YOU ACCEPT AND AGREE TO BE BOUND BY THE TERMS
 * OF THIS LICENSE. TO THE EXTENT THIS LICENSE MAY BE CONSIDERED TO BE A CONTRACT, THE LICENSOR GRANTS
 * YOU THE RIGHTS CONTAINED HERE IN CONSIDERATION OF YOUR ACCEPTANCE OF SUCH TERMS AND CONDITIONS.
 */
#include "encoding.h"
#include "wchar.h"

/*
*u16* wcscat(u16 *dst, const u16 *src) - appends one wchar_t string onto another.
*
*Purpose:
*       wcscat() concatenates (appends) a copy of the source string to the
*       end of the destination string, returning the destination string.
*       Strings are wide-character strings.
*/
u16* wcscat(u16 *dst, const u16 *src)
{
	u16 * cp=dst;
	/* find end of dst */
	while(*cp) cp++;
	/* Copy src to end of dst */
	while(*cp++=*src++);
	/* return dst */
	return dst;
}

/*
*wchar_t *wcsncat(front, back, count) - append count chars of back onto front
*
*Purpose:
*       Appends at most count characters of the string back onto the
*       end of front, and ALWAYS terminates with a null character.
*       If count is greater than the length of back, the length of back
*       is used instead.  (Unlike wcsncpy, this routine does not pad out
*       to count characters).
*
*Entry:
*       wchar_t *front - string to append onto
*       wchar_t *back - string to append
*       size_t count - count of max characters to append
*
*Exit:
*       returns a pointer to string appended onto (front).
*/
u16* wcsncat(u16* front, const u16* back, size_t count)
{
	u16 *start = front;

	while (*front++);
	front--;

	while (count--)
		if (!(*front++ = *back++))
			return(start);
	*front = L'\0';

	return start;
}

/*
*wcscmp - compare two wchar_t strings,
*        returning less than, equal to, or greater than
*
*Purpose:
*       wcscmp compares two wide-character strings and returns an integer
*       to indicate whether the first is less than the second, the two are
*       equal, or whether the first is greater than the second.
*
*       Comparison is done wchar_t by wchar_t on an UNSIGNED basis, which is to
*       say that Null wchar_t(0) is less than any other character.
*
*Entry:
*       const wchar_t * src - string for left-hand side of comparison
*       const wchar_t * dst - string for right-hand side of comparison
*
*Exit:
*       returns -1 if src <  dst
*       returns  0 if src == dst
*       returns +1 if src >  dst
*/
int wcscmp(const u16 *src, const u16 *dst)
{
	int ret=0;

	while(!(ret=(int)(*src-*dst)) && *dst) ++src, ++dst;

	if(ret<0) ret=-1;
	else if(ret>0) ret=1;

	return ret;
}

/*
*int wcsncmp(first, last, count) - compare first count chars of wchar_t strings
*
*Purpose:
*       Compares two strings for lexical order.  The comparison stops
*       after: (1) a difference between the strings is found, (2) the end
*       of the strings is reached, or (3) count characters have been
*       compared (wide-character strings).
*
*Entry:
*       wchar_t *first, *last - strings to compare
*       size_t count - maximum number of characters to compare
*
*Exit:
*       returns <0 if first < last
*       returns  0 if first == last
*       returns >0 if first > last
*/
int wcsncmp(const u16* first, const u16* last, size_t count)
{
	if(!count) return 0;

	while(--count && *first && *first == *last)
	{
		first++;
		last++;
	}

	return (int)(*first - *last);
}

/*
*       wcscpy() copies one wchar_t string into another.
*
*       wcscpy() copies the source string to the spot pointed to be
*       the destination string, returning the destination string.
*       Strings are wide-character strings.
*/
u16* wcscpy(u16 * dst, const u16 * src)
{
	u16 *cp=dst;
	/* Copy src over dst */
	while(*cp++=*src++);

	return( dst );
}

/*
*wchar_t *wcsncpy(dest, source, count) - copy at most n wide characters
*
*Purpose:
*       Copies count characters from the source string to the
*       destination.  If count is less than the length of source,
*       NO NULL CHARACTER is put onto the end of the copied string.
*       If count is greater than the length of sources, dest is padded
*       with null characters to length count (wide-characters).
*
*
*Entry:
*       wchar_t *dest - pointer to destination
*       wchar_t *source - source string for copy
*       size_t count - max number of characters to copy
*
*Exit:
*       returns dest
*/
u16* wcsncpy(u16* dest, const u16* source, size_t count)
{
	u16 *start = dest;

	/* copy string */
	while (count && (*dest++ = *source++)) count--;

	/* pad out with zeroes */
	if (count) while (--count) *dest++ = L'\0';

	return start;
}

/*
*wcslen - return the length of a null-terminated wide-character string
*
*Purpose:
*       Finds the length in wchar_t's of the given string, not including
*       the final null wchar_t (wide-characters).
*
*Entry:
*       const wchar_t * wcs - string whose length is to be computed
*
*Exit:
*       length of the string "wcs", exclusive of the final null wchar_t
*/
size_t wcslen(const u16 *wcs)
{
	const u16 *eos=wcs;

	while(*eos++);

	return (size_t)(eos-wcs-1);
}

/*
*wchar_t *wcschr(string, c) - search a string for a wchar_t character
*
*Purpose:
*       Searches a wchar_t string for a given wchar_t character,
*       which may be the null character L'\0'.
*
*Entry:
*       wchar_t *string - wchar_t string to search in
*       wchar_t c - wchar_t character to search for
*
*Exit:
*       returns pointer to the first occurence of c in string
*       returns NULL if c does not occur in string
*/
u16* wcschr(const u16* string, u16 ch)
{
	while (*string && *string != (u16)ch) string++;

	if (*string == (u16)ch) return (u16 *)string;

	return NULL;
}

/*
*wchar_t *wcsrchr(string, ch) - find last occurrence of ch in wide string
*
*Purpose:
*       Finds the last occurrence of ch in string.  The terminating
*       null character is used as part of the search (wide-characters).
*
*Entry:
*       wchar_t *string - string to search in
*       wchar_t ch - character to search for
*
*Exit:
*       returns a pointer to the last occurrence of ch in the given
*       string
*       returns NULL if ch does not occurr in the string
*/
u16* wcsrchr(const u16* string, u16 ch)
{
	u16 *start = (u16*)string;

	/* find end of string */
	while (*string++);                       
	/* search towards front */
	while (--string != start && *string != (u16)ch);
	/* wchar_t found ? */
	if (*string == (u16)ch) return( (u16*)string );

	return NULL;
}

/*
*wchar_t *wcspbrk(string, control) - scans string for a character from control
*
*Purpose:
*       Returns pointer to the first wide-character in
*       a wide-character string in the control string.
*
*Entry:
*       wchar_t *string - string to search in
*       wchar_t *control - string containing characters to search for
*
*Exit:
*       returns a pointer to the first character from control found
*       in string.
*       returns NULL if string and control have no characters in common.
*/
u16* wcspbrk(const u16* string, const u16* control)
{
	u16 *wcset;

	/* 1st char in control string stops search */
	while (*string)
	{
		for (wcset = (u16 *) control; *wcset; wcset++)
		{
			if (*wcset == *string)
			{
				return (u16*) string;
			}
		}
		string++;
	}

	return NULL;
}

/*
*int wcsspn(string, control) - find init substring of control chars
*
*Purpose:
*       Finds the index of the first character in string that does belong
*       to the set of characters specified by control.  This is
*       equivalent to the length of the initial substring of string that
*       consists entirely of characters from control.  The L'\0' character
*       that terminates control is not considered in the matching process
*       (wide-character strings).
*
*Entry:
*       wchar_t *string - string to search
*       wchar_t *control - string containing characters not to search for
*
*Exit:
*       returns index of first wchar_t in string not in control
*/
size_t wcsspn(const u16* string, const u16* control)
{
	u16 *str = (u16 *) string;
	u16 *ctl;

	/* 1st char not in control string stops search */
	while (*str)
	{
		for (ctl = (u16 *)control; *ctl != *str; ctl++)
		{
			if (*ctl == (u16)0)
			{
				/*
				* reached end of control string without finding a match
				*/
				return (size_t)(str - string);
			}
		}
		str++;
	}
	/*
	* The whole string consisted of characters from control
	*/
	return (size_t)(str - string);
}

/*
*size_t wcscspn(string, control) - search for init substring w/o control wchars
*
*Purpose:
*       returns the index of the first character in string that belongs
*       to the set of characters specified by control.  This is equivalent
*       to the length of the length of the initial substring of string
*       composed entirely of characters not in control.  Null chars not
*       considered (wide-character strings).
*
*Entry:
*       wchar_t *string - string to search
*       wchar_t *control - set of characters not allowed in init substring
*
*Exit:
*       returns the index of the first wchar_t in string
*       that is in the set of characters specified by control.
*/
size_t wcscspn(const u16* string, const u16* control)
{
	u16 *str = (u16 *) string;
	u16 *wcset;

	/* 1st char in control string stops search */
	while (*str)
	{
		for (wcset = (u16 *)control; *wcset; wcset++)
			if (*wcset == *str)
				return (size_t)(str - string);
		str++;
	}
	return (size_t)(str - string);
}

/*
*wchar_t *wcstok(string, control) - tokenize string with delimiter in control
*       (wide-characters)
*
*Purpose:
*       wcstok considers the string to consist of a sequence of zero or more
*       text tokens separated by spans of one or more control chars. the first
*       call, with string specified, returns a pointer to the first wchar_t of
*       the first token, and will write a null wchar_t into string immediately
*       following the returned token. subsequent calls with zero for the first
*       argument (string) will work thru the string until no tokens remain. the
*       control string may be different from call to call. when no tokens remain
*       in string a NULL pointer is returned. remember the control chars with a
*       bit map, one bit per wchar_t. the null wchar_t is always a control char
*       (wide-characters).
*
*Entry:
*       wchar_t *string - wchar_t string to tokenize, or NULL to get next token
*       wchar_t *control - wchar_t string of characters to use as delimiters
*
*Exit:
*       returns pointer to first token in string, or if string
*       was NULL, to next token
*       returns NULL when no more tokens remain.
*/
u16* wsctok(u16 *s1, const u16* delimit)
{
    static u16 *lastToken = NULL; /* UNSAFE SHARED STATE! */
    u16 *tmp;

    /* Skip leading delimiters if new string. */
    if ( s1 == NULL )
	{
        s1 = lastToken;
        if (s1 == NULL)         /* End of story? */
            return NULL;
    }
	else s1 += wcsspn(s1, delimit);

    /* Find end of segment */
    tmp = wcspbrk(s1, delimit);
    if (tmp)
	{
        /* Found another delimiter, split string and save state. */
        *tmp = '\0';
        lastToken = tmp + 1;
    }
	/* Last segment, remember that. */
	else lastToken = NULL;

    return s1;
}

/*
*wchar_t *wcsstr(string1, string2) - search for string2 in string1
*       (wide strings)
*
*Purpose:
*       finds the first occurrence of string2 in string1 (wide strings)
*
*Entry:
*       wchar_t *string1 - string to search in
*       wchar_t *string2 - string to search for
*
*Exit:
*       returns a pointer to the first occurrence of string2 in
*       string1, or NULL if string2 does not occur in string1
*/
u16* wcsstr(const u16* wcs1, const u16* wcs2)
{
	u16 *cp = (u16 *) wcs1;
	u16 *s1, *s2;

	if ( !*wcs2) return (u16 *)wcs1;

	while (*cp)
	{
		s1 = cp;
		s2 = (u16 *) wcs2;

		while ( *s1 && *s2 && !(*s1-*s2) ) s1++, s2++;

		if (!*s2) return(cp);

		cp++;
	}

	return NULL;
}
