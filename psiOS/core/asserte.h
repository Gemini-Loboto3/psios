#ifndef	__ASSERTE_H
#define	__ASSERTE_H

# ifdef NDEBUG
# define _asserte(x)
# define asserte(x)
# else

# define _asserte(x)	{if (!(x)){DEBUGPRINT(("Assertion failed: file \"%s\", line %d\n", __FILE__, __LINE__));}}
# define asserte(x)		_asserte(x)

# endif

#endif	// __ASSERTE_H