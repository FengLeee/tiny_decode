/*
 * common.h
 *
 *  Created on : 6/10/2014
 *      Author : zhiming.wang
 */

# ifndef _COMMON_H_
# define _COMMON_H_
//# include <vxWorks.h>
# include <stdlib.h>
# include <stdio.h>
# include <assert.h>
# include <math.h>
# include <cmath>
# include <string.h>
# include <stdint.h>
# include <limits.h>
# include <float.h>
# include <time.h>
//# include <memlib.h>

/* add "-lstdc++ -lm" as gcc link options */

typedef float BaseFloat ;


//# define INFINITY  FLT_MAX

/*  integer and float type definition  */
typedef uint16_t uint16 ;
typedef uint32_t uint32 ;
typedef uint64_t uint64 ;
typedef int16_t int16 ;
typedef int32_t int32 ;
typedef int64_t int64 ;

typedef float float32 ;
typedef double double64 ;

# define PI 3.14159265358979323846f
# define PI2 6.283185307179586476925286766559005f

int isnan(float x);
int isinf(float x);
int isfinite(float x);

//int isnan(double x);
//int isinf(double x);
//int isfinite(double x);

# define ISNAN(x)  isnan(x)           /*  return a non-zero value if x is "not a number"  */
# define ISINF(x)  isinf(x)           /*  return 1 if x is positive infinity , and -1 if x is negative infinity  */
# define ISFINITE(x)  isfinite(x)     /*  return a non-zero value if x is a number and finite  */
//# define ISNORMAL(x)  isnormal(x)     /*  return a non-zero value if x is a normal floating-point number , e.g. , it is not too small to be represented in normalized format  */


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

/* round up to nearest power of two  */
uint32 round_upto_nearest_power_of_two(uint32 n) ;

/*  this function is for error information  */
void error(const char* error_message , const char* file , uint32 line) ;

/* allocate address */
void Malloc(BaseFloat** ptr , size_t size) ;
void Malloc(int32** ptr , size_t size) ;

/* free address */
void Free(BaseFloat** ptr) ;
void Free(int32** ptr) ;

void CopyStr(char** dst , const char* src) ;

void FreeStr(char** str) ;

/* sort */
int32 Divide(BaseFloat* X , int32 Low , int32 High) ;

void QuickSort(BaseFloat* X , int32 Start , int32 End) ;

void nthElement(BaseFloat* X , int32 Start , int32 End , int32 N) ;

void RandomShuffle(BaseFloat* X , int32 Length) ;



# include <sys/time.h>


# endif /*  _COMMON_H_  */
