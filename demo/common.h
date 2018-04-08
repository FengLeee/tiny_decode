/*
 * common.h
 *
 *  Created on : 6/10/2014
 *      Author : zhiming.wang
 */

# ifndef _COMMON_H_
# define _COMMON_H_

# include <stdlib.h>
# include <stdio.h>
# include <assert.h>
# include <math.h>
# include <string.h>
# include <stdint.h>

/*  integer and float type definition  */
typedef uint16_t uint16 ;
typedef uint32_t uint32 ;
typedef uint64_t uint64 ;
typedef int16_t int16 ;
typedef int32_t int32 ;
typedef int64_t int64 ;

typedef float float32 ;
typedef double double64 ;

typedef float BaseFloat ;

# define ISNAN(x) isnan(x)           /*  return a non-zero value if x is "not a number"  */
# define ISINF(x) isinf(x)           /*  return 1 if x is positive infinity , and -1 if x is negative infinity  */
# define ISFINITE(x) isfinite(x)     /*  return a non-zero value if x is a number and finite  */
# define ISNORMAL(x) isnormal(x)     /*  return a non-zero value if x is a normal floating-point number , e.g. , it is not too small to be represented in normalized format  */

# define MAX(x , y) (((x) > (y)) ? (x) : (y))
# define MIN(x , y) (((x) > (y)) ? (y) : (x))

/*  consider big-endian and little-endian and convert between them  */
# define SWAP8(a)  \
    {  \
		int t = ((char*)a)[0] ; ((char*)a)[0] = ((char*)a)[7] ; ((char*)a)[7] = t ;  \
			t = ((char*)a)[1] ; ((char*)a)[1] = ((char*)a)[6] ; ((char*)a)[6] = t ;  \
			t = ((char*)a)[2] ; ((char*)a)[2] = ((char*)a)[5] ; ((char*)a)[5] = t ;  \
			t = ((char*)a)[3] ; ((char*)a)[3] = ((char*)a)[4] ; ((char*)a)[4] = t ;  \
    }

# define SWAP4(a)  \
	{   \
		int t = ((char*)a)[0] ; ((char*)a)[0] = ((char*)a)[3] ; ((char*)a)[3] = t ;  \
			t = ((char*)a)[1] ; ((char*)a)[1] = ((char*)a)[2] ; ((char*)a)[2] = t ;  \
	}

# define SWAP2(a)  \
	{  \
		int t = ((char*)a)[0] ; ((char*)a)[0] = ((char*)a)[1] ; ((char*)a)[1] = t ;  \
	}

/* read wave , and store the the actual sound data */
int wav_read(const char* wave , BaseFloat** data , uint32* length) ;

# endif /*  _COMMON_H_  */
