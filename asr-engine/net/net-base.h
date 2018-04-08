/*
 * net-base.h
 *
 *  Created on : 07/24/2014
 *      Author : zhiming.wang
 */

# ifndef _NET_BASE_H_
# define _NET_BASE_H_

# include "base/common.h"

# ifndef __ARM_NEON__
# define __AVX__
# endif

# define ALIGN_BOUNDARY 32

/* SIMD operation */
/* in support of AVX (Intel Advanced Vector Extensions) instructions */
/* add "-march=corei7-avx" as compile options */
# if defined __AVX__

# include <immintrin.h>

# if defined ALIGN_BOUNDARY
# undef ALIGN_BOUNDARY
# endif

# define ALIGN_BOUNDARY sizeof(__m256i)

# endif  /* __AVX__ */

/* in support of ARM NEON instructions */
/* add "-march=armv7-a -mfpu=neon -ftree-vectorize -mfloat-abi=softfp" as compile options */
# if defined __ARM_NEON__

# include <arm_neon.h>

# endif  /* __ARM_NEON__ */

/* check memory alignment */
# define is_aligned(ptr , align_boundary)  (0 == ((uintptr_t)(const void *)(ptr)) % (align_boundary))

/* allocate aligned address */
void AlignedAlloc(BaseFloat** ptr , size_t size) ;

/* free aligned address */
void AlignedFree(BaseFloat** ptr) ;

/* matrix multiplication */
/*
 * A : M * K
 * B : N * K
 * C : A * B , M * N
 */
void MatrixMul(const int32 M , const int32 N , const int32 K , const BaseFloat* A , const BaseFloat* B , BaseFloat* C) ;

# endif /* _NET_BASE_H_ */