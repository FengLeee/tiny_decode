/*
 * net-base.cc
 *
 *  Created on : 07/24/2014
 *      Author : zhiming.wang
 */

# include "net/net-base.h"

/* allocate aligned address */
void AlignedAlloc(BaseFloat** ptr , size_t size)
{
# if defined __linux__
	if(0 != posix_memalign((void**)ptr , ALIGN_BOUNDARY , size))
		error("memory allocation error , unable to allocate aligned memory using posix_memalign" , __FILE__ , __LINE__) ;
# elif defined __MINGW32__
	*ptr = (BaseFloat*)__mingw_aligned_malloc(size , ALIGN_BOUNDARY) ;
	if(NULL == *ptr)  error("memory allocation error , unable to allocate aligned memory using __mingw_aligned_malloc" , __FILE__ , __LINE__) ;
# else
	*ptr = (BaseFloat*)malloc(size) ;
	if(NULL == *ptr)  error("memory allocation error , unable to allocate aligned memory using malloc" , __FILE__ , __LINE__) ;
# endif
}

/* free aligned address */
void AlignedFree(BaseFloat** ptr)
{
	if(NULL != *ptr)
	{
# if defined __MINGW32__
		__mingw_aligned_free(*ptr) ;
# else
		free(*ptr) ;
# endif
		*ptr = NULL ;
	}
}

# if defined __ARM_NEON__

# include <pthread.h>
# include <sys/sysinfo.h>
# include <unistd.h>

/*
 * A : M * K
 * B : N * K
 * C : B * A , N * M
 * M , N , K : multiple of 4
 */
static void MatMulMat(const int M , const int N , const int K , const float* A , const int lda , const float* B , const int ldb , float* C , const int ldc)
{
	int i , j , k ;

	register double D24 asm ("d24") ;
	register double D25 asm ("d25") ;
	register double D26 asm ("d26") ;
	register double D27 asm ("d27") ;
	register double D28 asm ("d28") ;
	register double D29 asm ("d29") ;
	register double D30 asm ("d30") ;
	register double D31 asm ("d31") ;

	const int inca = 4 * lda , jnca = 16 - 12 * lda , incb = 4 * ldb , jncb = 16 - 12 * ldb ;
	const int incc = 4 * ldc ;
	
	float *pA , *pB , *pC , *pCtemp ;
	
	for(j = 0 ; j < N ; j += 4)
	{
		for(i = 0 ; i < M ; i += 4)
		{
			pA = (float*)A + i * lda ;
			pB = (float*)B + j * ldb ;
			pC = (float*)C + i + j * ldc ;

			pCtemp = pC ;
			__asm__ __volatile__(
				/*  load zeros into q12-q15(C)  */
				"vmov.f32                          q12 ,          #0.0               \n\t"
				"vmov.f32                          q13 ,          #0.0               \n\t"
				"vmov.f32                          q14 ,          #0.0               \n\t"
				"vmov.f32                          q15 ,          #0.0               \n\t"

				: [pCtemp] "+r" (pCtemp) , "=w" (D24) , "=w" (D25) , "=w" (D26) , "=w" (D27) , "=w" (D28) , "=w" (D29) , "=w" (D30) , "=w" (D31)
				: [incc] "r" (incc)
				: "s0"
	        ) ;

	        for(k = 0 ; k < K ; k += 4)
	        {
	        	__asm__ __volatile__(
	                /*  load the next block of A.Transposition into q8-q11  */
	                "vld4.32 {d16[0],d18[0],d20[0],d22[0]} ,       [%[pA]] ,       %[inca]\n\t"
	                "vld4.32 {d16[1],d18[1],d20[1],d22[1]} ,       [%[pA]] ,       %[inca]\n\t"
	                "vld4.32 {d17[0],d19[0],d21[0],d23[0]} ,       [%[pA]] ,       %[inca]\n\t"
	                "vld4.32 {d17[1],d19[1],d21[1],d23[1]} ,       [%[pA]] ,       %[jnca]\n\t"
	        	  /*  load the next block of B into q4-q7  */
	        	  "vld1.32                       {d8-d9} ,       [%[pB]] ,       %[incb]\n\t"
	        	  "vld1.32                     {d10-d11} ,       [%[pB]] ,       %[incb]\n\t"
	        	  "vld1.32                     {d12-d13} ,       [%[pB]] ,       %[incb]\n\t"
	        	  "vld1.32                     {d14-d15} ,       [%[pB]] ,       %[jncb]\n\t"
	                /*  multiply q4-q7 (B) by q8-q11 (A) and add the result to q12-q15(C)  */
	                "vmla.f32                          q12 ,            q8 ,        d8[0]\n\t"
	                "vmla.f32                          q13 ,            q8 ,        d10[0]\n\t"
	                "vmla.f32                          q14 ,            q8 ,        d12[0]\n\t"
	                "vmla.f32                          q15 ,            q8 ,        d14[0]\n\t"
	                "vmla.f32                          q12 ,            q9 ,        d8[1]\n\t"
	                "vmla.f32                          q13 ,            q9 ,        d10[1]\n\t"
	                "vmla.f32                          q14 ,            q9 ,        d12[1]\n\t"
	                "vmla.f32                          q15 ,            q9 ,        d14[1]\n\t"
	                "vmla.f32                          q12 ,           q10 ,        d9[0]\n\t"
	                "vmla.f32                          q13 ,           q10 ,        d11[0]\n\t"
	                "vmla.f32                          q14 ,           q10 ,        d13[0]\n\t"
	                "vmla.f32                          q15 ,           q10 ,        d15[0]\n\t"
	                "vmla.f32                          q12 ,           q11 ,        d9[1]\n\t"
	                "vmla.f32                          q13 ,           q11 ,        d11[1]\n\t"
	                "vmla.f32                          q14 ,           q11 ,        d13[1]\n\t"
	                "vmla.f32                          q15 ,           q11 ,        d15[1]\n\t"
	                : [pA] "+r" (pA) , [pB] "+r" (pB) , "+w" (D24) , "+w" (D25) , "+w" (D26) , "+w" (D27) , "+w" (D28) , "+w" (D29) , "+w" (D30) , "+w" (D31)
	                : [inca] "r" (inca) , [jnca] "r" (jnca) , [incb] "r" (incb) , [jncb] "r" (jncb)
	                : "d8" , "d9" , "d10" , "d11" , "d12" , "d13" , "d14" , "d15" , "d16" , "d17" , "d18" , "d19" , "d20" , "d21" , "d22" , "d23"
	            ) ;
	        }

	        __asm__ __volatile__(
	            /*  C is real , write the result out from q12-q15  */
	            "vst1.32                     {d24-d25} ,       [%[pC]] ,       %[incc]\n\t"
	            "vst1.32                     {d26-d27} ,       [%[pC]] ,       %[incc]\n\t"
	            "vst1.32                     {d28-d29} ,       [%[pC]] ,       %[incc]\n\t"
	            "vst1.32                     {d30-d31} ,       [%[pC]]                \n\t"
	            : [pC] "+r" (pC)
	            : [incc] "r" (incc) , "w" (D24) , "w" (D25) , "w" (D26) , "w" (D27) , "w" (D28) , "w" (D29) , "w" (D30) , "w" (D31)
	            : "memory"
	        ) ;
	    }
	}
}

struct Multiplier 
{
	const float* A ;
	const float* B ;
	float* C ;
	int M ;
	int N ;
	int K ;
} ;

void* ARMMatMutMat(void* param)
{
	struct Multiplier* multiplier = (struct Multiplier*)(param) ;

	MatMulMat(multiplier->M , multiplier->N , multiplier->K , multiplier->A , multiplier->K , multiplier->B , multiplier->K , multiplier->C , multiplier->M) ;

	return NULL ;
}

# endif

/* matrix multiplication */
/*
 * A : M * K
 * B : N * K
 * C : A * B , M * N
 */
void MatrixMul(const int32 M , const int32 N , const int32 K , const BaseFloat* A , const BaseFloat* B , BaseFloat* C)
{
# if defined __ARM_NEON__
	assert(is_aligned(A , ALIGN_BOUNDARY)) ;
	assert(is_aligned(B , ALIGN_BOUNDARY)) ;
	
	/* M , N , K should be multiple of 4 */
	/* otherwise , padding to be multiple of 4 */
	const int32 number_thread = 2 * get_nprocs() ; 
	int32 M_ = M , N_ = N , K_ = K ;
	while(0 != (M_ % (number_thread * 4)))  M_++ ;
	while(0 != (N_ % 4))  N_++ ;
	while(0 != (K_ % 4))  K_++ ;
	
	BaseFloat *A_ = NULL , *B_ = NULL , *C_ = NULL ;
	
	if(M_ != M || K_ != K)
	{
		AlignedAlloc(&A_ , sizeof(BaseFloat) * M_ * K_) ;
		if(K_ != K)
		{
			for(int32 i = 0 ; i < M ; i++)
			{
				memcpy((void*)(A_ + i * K_) , (const void*)(A + i * K) , K * sizeof(BaseFloat)) ;
				memset((void*)(A_ + i * K_ + K) , 0 , (K_ - K) * sizeof(BaseFloat)) ;
			}
		} else
			memcpy((void*)A_ , (const void*)A , M * K * sizeof(BaseFloat)) ;

		if(M_ != M)
			memset((void*)(A_ + M * K_) , 0 , (M_ - M) * K_ * sizeof(BaseFloat)) ;
	} else
		A_ = (BaseFloat*)A ;

	if(N_ != N || K_ != K)
	{
		AlignedAlloc(&B_ , sizeof(BaseFloat) * N_ * K_) ;
		if(K_ != K)
		{
			for(int32 i = 0 ; i < N ; i++)
			{
				memcpy((void*)(B_ + i * K_) , (const void*)(B + i * K) , K * sizeof(BaseFloat)) ;
				memset((void*)(B_ + i * K_ + K) , 0 , (K_ - K) * sizeof(BaseFloat)) ;
			}
		} else
			memcpy((void*)B_ , (const void*)B , N * K * sizeof(BaseFloat)) ;

		if(N_ != N)
			memset((void*)(B_ + N * K_) , 0 , (N_ - N) * K_ * sizeof(BaseFloat)) ;
	} else
		B_ = (BaseFloat*)B ;

	if(M_ != M || N_ != N)
		AlignedAlloc(&C_ , sizeof(BaseFloat) * M_ * N_) ;
	else
		C_ = C ;
	
	int32 C_Offset = (M_ / number_thread) * N_ , A_Offset = (M_ / number_thread) * K_ ;

	struct Multiplier* multiplier_ = (struct Multiplier*)malloc(number_thread * sizeof(struct Multiplier)) ;
	pthread_t* threadHandle_ = (pthread_t*)malloc(number_thread * sizeof(pthread_t)) ;
	assert(NULL != multiplier_ && NULL != threadHandle_) ;
	
	for(int32 i = 0 ; i < number_thread ; i++)
	{
		multiplier_[i].M = N_ ;
		multiplier_[i].N = M_ / number_thread ;
		multiplier_[i].K = K_ ; 
		multiplier_[i].A = B_ ;
		multiplier_[i].B = A_ + i * A_Offset ;
		multiplier_[i].C = C_ + i * C_Offset ;

		pthread_create(&threadHandle_[i] , NULL , ARMMatMutMat , (void*)&multiplier_[i]) ;
		
		usleep(5) ;
	}
	
	for(int32 i = 0 ; i < number_thread ; i++)  
		pthread_join(threadHandle_[i] , NULL) ; 

	if(M_ != M || N_ != N)
	{
		for(int32 i = 0 ; i < M ; i++)
			memcpy((void*)(C + i * N) , (const void*)(C_ + i * N_) , N * sizeof(BaseFloat)) ;

		AlignedFree(&C_) ;
	}
	
	free(multiplier_) ; multiplier_ = NULL ;
	free(threadHandle_) ; threadHandle_ = NULL ;

	if(M_ != M || K_ != K)  AlignedFree(&A_) ;
	if(N_ != N || K_ != K)  AlignedFree(&B_) ;
# elif defined __AVX__
	assert(is_aligned(A , ALIGN_BOUNDARY)) ;
	assert(is_aligned(B , ALIGN_BOUNDARY)) ;

	/* K should be multiple of 8 */
	int32 K_ = K ;
	while(0 != (K_ % 8))  K_++ ;
	BaseFloat *A_ = NULL , *B_ = NULL ;
	if(K_ != K)
	{
		AlignedAlloc(&A_ , sizeof(BaseFloat) * M * K_) ;
		AlignedAlloc(&B_ , sizeof(BaseFloat) * N * K_) ;

		for(int32 i = 0 ; i < M ; i++)
		{
			memcpy((void*)(A_ + i * K_) , (const void*)(A + i * K) , K * sizeof(BaseFloat)) ;
			memset((void*)(A_ + i * K_ + K) , 0 , (K_ - K) * sizeof(BaseFloat)) ;
		}

		for(int32 i = 0 ; i < N ; i++)
		{
			memcpy((void*)(B_ + i * K_) , (const void*)(B + i * K) , K * sizeof(BaseFloat)) ;
			memset((void*)(B_ + i * K_ + K) , 0 , (K_ - K) * sizeof(BaseFloat)) ;
		}
	} else {
		A_ = (BaseFloat*)A ;
		B_ = (BaseFloat*)B ;
	}

	BaseFloat *ptr_A = NULL , *ptr_B = NULL , *ptr_result = NULL ;

	__m256 reg_A , reg_B , reg_mul_result , reg_acc ;

	const int32 block_width = 8 * 8 ;
	int32 num_block = K_ / block_width , num_remain = K_ % block_width ;
	BaseFloat result = 0.0 ;

	for(int32 i = 0 ; i < M ; i++)
		for(int32 j = 0 ; j < N ; j++)
		{
			ptr_A = A_ + i * K_ ;
			ptr_B = B_ + j * K_ ;
			reg_acc = _mm256_setzero_ps() ;

			for(int32 k = 0 ; k < num_block ; ++k)
			{
				reg_A = _mm256_load_ps(ptr_A) ;
				reg_B = _mm256_load_ps(ptr_B) ;
				reg_mul_result = _mm256_mul_ps(reg_A , reg_B) ;
				reg_acc = _mm256_add_ps(reg_acc , reg_mul_result ) ;

				reg_A = _mm256_load_ps(ptr_A + 8) ;
				reg_B = _mm256_load_ps(ptr_B + 8) ;
				reg_mul_result = _mm256_mul_ps(reg_A , reg_B) ;
				reg_acc = _mm256_add_ps(reg_acc , reg_mul_result ) ;

				reg_A = _mm256_load_ps(ptr_A + 16) ;
				reg_B = _mm256_load_ps(ptr_B + 16) ;
				reg_mul_result = _mm256_mul_ps(reg_A , reg_B) ;
				reg_acc = _mm256_add_ps(reg_acc , reg_mul_result ) ;

				reg_A = _mm256_load_ps(ptr_A + 24) ;
				reg_B = _mm256_load_ps(ptr_B + 24) ;
				reg_mul_result = _mm256_mul_ps(reg_A , reg_B) ;
				reg_acc = _mm256_add_ps(reg_acc , reg_mul_result ) ;

				reg_A = _mm256_load_ps(ptr_A + 32) ;
				reg_B = _mm256_load_ps(ptr_B + 32) ;
				reg_mul_result = _mm256_mul_ps(reg_A , reg_B) ;
				reg_acc = _mm256_add_ps(reg_acc , reg_mul_result ) ;

				reg_A = _mm256_load_ps(ptr_A + 40) ;
				reg_B = _mm256_load_ps(ptr_B + 40) ;
				reg_mul_result = _mm256_mul_ps(reg_A , reg_B) ;
				reg_acc = _mm256_add_ps(reg_acc , reg_mul_result ) ;

				reg_A = _mm256_load_ps(ptr_A + 48) ;
				reg_B = _mm256_load_ps(ptr_B + 48) ;
				reg_mul_result = _mm256_mul_ps(reg_A , reg_B) ;
				reg_acc = _mm256_add_ps(reg_acc , reg_mul_result ) ;

				reg_A = _mm256_load_ps(ptr_A + 56) ;
				reg_B = _mm256_load_ps(ptr_B + 56) ;
				reg_mul_result = _mm256_mul_ps(reg_A , reg_B) ;
				reg_acc = _mm256_add_ps(reg_acc , reg_mul_result ) ;

				ptr_A += block_width ;  ptr_B += block_width ;
			}

			ptr_result = (BaseFloat*)(&reg_acc) ;
			result = ptr_result[0] + ptr_result[1] + ptr_result[2] + ptr_result[3] + ptr_result[4] + ptr_result[5] + ptr_result[6] + ptr_result[7] ;

			for(int32 k = 0 ; k < num_remain ; ++k)  result += ptr_A[k] * ptr_B[k] ;

			C[i * N + j] = result ;
		}

		if(K_ != K)
		{
			AlignedFree(&A_) ;
			AlignedFree(&B_) ;
		}
# else
	BaseFloat result = 0.0 ;

	for(int32 i = 0 ; i < M ; i++)
		for(int32 j = 0 ; j < N ; j++)
		{
			result = 0.0 ;
			for(int k = 0 ; k < K ; ++k)  result += A[i * K + k] * B[ j* K + k] ;
			C[i * N + j] = result ;
		}
# endif
}