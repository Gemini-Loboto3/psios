#ifndef __FIXED_H
#define __FIXED_H

// how many bits a decimal value con hold
#define FIXED_BITS	12
// mask for getting the decimal part
#define FIXED_MASK	((1<<(FIXED_BITS))-1)
//
#define FIXED_MULT	(1<<(FIXED_BITS))

// declaring just in case
typedef int fixed;

__inline static fixed getFixedInt(fixed val)
{
	return val>>FIXED_BITS;
}

__inline static fixed getFixedDec(fixed val)
{
	return val&FIXED_MASK;
}

#endif
