/*
 * common.cc
 *
 *  Created on : 6/10/2014
 *      Author : zhiming.wang
 */

# include "common.h"

/*  round up to nearest power of two  */
uint32 round_upto_nearest_power_of_two(uint32 n)
{
	assert(n > 0) ;
	n-- ;
	n |= n >> 1 ;
	n |= n >> 2 ;
	n |= n >> 4 ;
	n |= n >> 8 ;
	n |= n >> 16 ;
	return n + 1 ;
}

/*  this function is for error information  */
void error(const char* error_message , const char* file , uint32 line)
{
	printf("%s , %s : line %u \n" , error_message , file , line) ;
	assert(NULL) ;
}

/* allocate address of given size */
void Malloc(BaseFloat** ptr , size_t size)
{
	*ptr = (BaseFloat*)malloc(size) ;
	if(NULL == *ptr)  error("memory allocation error , unable to allocate memory using malloc" , __FILE__ , __LINE__) ;
}

void Malloc(int32** ptr , size_t size)
{
	*ptr = (int32*)malloc(size) ;
	if(NULL == *ptr)  error("memory allocation error , unable to allocate memory using malloc" , __FILE__ , __LINE__) ;
}

/* free address */
void Free(BaseFloat** ptr)
{
	if(NULL != *ptr)
	{
		free(*ptr) ;
		*ptr = NULL ;
	}
}

void Free(int32** ptr)
{
	if(NULL != *ptr)
	{
		free(*ptr) ;
		*ptr = NULL ;
	}
}

void CopyStr(char** dst , const char* src)
{
	size_t len = strlen(src) ;
	*dst = (char*)malloc(sizeof(char) * (len + 1)) ;
	assert(NULL != *dst) ;
	memcpy((void*)(*dst) ,(const void*)src , sizeof(char) * len) ;
	(*dst)[len] ='\0' ;
}

void FreeStr(char** str)
{
	if(NULL != *str)
	{
		free(*str) ;
		*str = NULL ;
	}
}

/* sort */
int32 Divide(BaseFloat* X , int32 Low , int32 High)
{
	BaseFloat Token = X[Low] ;

	while(Low < High)
	{
		while(Low < High && X[High] >= Token)  High-- ;
		X[Low] = X[High] ;

		while(Low < High && X[Low] <= Token)  Low++ ;
		X[High] = X[Low] ;
	}

	if(Low == High)  X[Low] = Token ;

	return Low ;
}

void QuickSort(BaseFloat* X , int32 Start , int32 End)
{
	if(Start >= End)  return ;

	int32 Partition = Divide(X , Start , End) ;

	QuickSort(X , Start , Partition - 1) ;

	QuickSort(X , Partition + 1 , End) ;
}

void nthElement(BaseFloat* X , int32 Start , int32 End , int32 N)
{
	if(Start >= End)  return ;

	int32 Partition = Divide(X , Start , End) ;

	if(Partition < N)
		nthElement(X , Partition + 1 , End , N) ;
	else if (Partition == N)
		return ;
	else
		nthElement(X , Start , Partition - 1 , N) ;
}

void RandomShuffle(BaseFloat* X , int32 Length)
{
	srand((unsigned int)(time(NULL))) ;

	for(int32 i = Length - 1 ; i > 0 ; --i)
	{
		int32 j = ((int32)rand()) % i ;

		BaseFloat Y = X[i] ;
		X[i] = X[j] ;
		X[j] = Y ;
	}
}

int isnan(float x) {return x != x;}
int isinf(float x) {return !isnan(x) && isnan(x-x);}
int isfinite(float x) {return !isinf(x);}

int isnan(double x) {return x != x;}
int isinf(double x) {return !isnan(x) && isnan(x-x);}
int isfinite(double x) {return !isinf(x);}
