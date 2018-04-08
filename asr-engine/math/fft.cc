/*
 * fft.cc
 *
 *  Created on : 6/10/2014
 *      Author : zhiming.wang
 */

# include "fft.h"

FFT::FFT(uint32 n) : butterfly_coefficient_real(NULL) , butterfly_coefficient_imag(NULL) , N(0)
{
	initialize(n) ;
}

FFT::~FFT()
{
	release() ;
}

/*  initialize  */
void FFT::initialize(uint32 n)
{
	if(0 != (n & (n - 1)))  error("dimension size should be power of 2" , __FILE__ , __LINE__) ;
	
	uint32 dim = n >> 1 ;
	N = n ;
	
	butterfly_coefficient_real = (BaseFloat*)malloc(sizeof(BaseFloat) * dim) ;
	butterfly_coefficient_imag = (BaseFloat*)malloc(sizeof(BaseFloat) * dim) ;
	if(NULL == butterfly_coefficient_real || NULL == butterfly_coefficient_imag)  error("no memory to be allocated" , __FILE__ , __LINE__) ;
	
	BaseFloat base_unit = -1.0f * 2.0f * PI / n ;
	BaseFloat angle ;
	
	for(uint32 i = 0 ; i < dim ; ++i)
	{
		angle = base_unit * i ;
		butterfly_coefficient_real[i] = cos(angle) ;
		butterfly_coefficient_imag[i] = sin(angle) ;
	}
}

/*  release  */
void FFT::release()
{
	if(NULL != butterfly_coefficient_real)
	{
		free(butterfly_coefficient_real) ;
		butterfly_coefficient_real = NULL ;
	}

	if(NULL != butterfly_coefficient_imag)
	{
		free(butterfly_coefficient_imag) ;
		butterfly_coefficient_imag = NULL ;
	}

	N = 0 ;
}

/*  fast Fourier transform algorithm */
void FFT::fft_compute(const BaseFloat* in_real , const BaseFloat* in_imag , BaseFloat* out_real , BaseFloat* out_imag , uint32 n) const
{
	if(0 != (n & (n - 1)))  error("dimension size should be power of 2" , __FILE__ , __LINE__) ;

	uint32 pow = 0 , m = 1 ;
	while(m < n)
	{
		m *= 2 ; pow++ ;
	}
	
	assert(n == N) ;

	assert(NULL != out_real && NULL != out_imag) ;
	memcpy((void*)out_real , (const void*)in_real , sizeof(BaseFloat) * n) ;
	if(NULL != in_imag)  memcpy((void*)out_imag , (const void*)in_imag , sizeof(BaseFloat) * n) ;
	else  memset((void*)out_imag , 0 , sizeof(BaseFloat) * n) ;

	/*  bit reverse  */
	const uint32 half_size = n >> 1 ;
	uint32 j = half_size , i = 0 , k = 0 ;
	BaseFloat real[2] , imag[2] ;
	uint32 dist[2] ;
	uint32 twiddle ;

	for(i = 1 ; i < n - 1 ; i++)
	{
		if(j > i)
		{
			/*  swap i and j  */
			real[0] = out_real[j] ;  out_real[j] = out_real[i] ;   out_real[i] = real[0] ;
			imag[0] = out_imag[j] ;  out_imag[j] = out_imag[i] ;   out_imag[i] = imag[0] ;
		}

		k = half_size ;

		while(j >= k)
		{
			j -= k ;  k = (k >> 1) ;
		}

		j += k ;
	}

	for(i = 1 ; i <= pow ; i++)
	{
		dist[0] = (1 << (i - 1)) ;
		dist[1] = dist[0] << 1 ;

		for(j = 0 ; j < dist[0] ; j++)
		{
			twiddle = j * (1 << (pow - i)) ;

			for(k = j ; k < n ; k += dist[1])
			{
				real[0] = out_real[k] ;
				imag[0] = out_imag[k] ;
				real[1] = out_real[k + dist[0]] ;
				imag[1] = out_imag[k + dist[0]] ;

				out_real[k] +=  (real[1] * butterfly_coefficient_real[twiddle] - imag[1] * butterfly_coefficient_imag[twiddle]) ;
				out_imag[k] +=  (real[1] * butterfly_coefficient_imag[twiddle] + imag[1] * butterfly_coefficient_real[twiddle]) ;
				out_real[k + dist[0]] = real[0] - (real[1] * butterfly_coefficient_real[twiddle] - imag[1] * butterfly_coefficient_imag[twiddle]) ;
				out_imag[k + dist[0]] = imag[0] - (real[1] * butterfly_coefficient_imag[twiddle] + imag[1] * butterfly_coefficient_real[twiddle]) ;
			}
		}
	}
	for(int i = 0; i < 10; i++)
	{
		out_real[i] *= 0.1;
		out_imag[i] *= 0.1;
	}
}

/*  inverse fast Fourier transform algorithm  */
void FFT::ifft_compute(const BaseFloat* in_real , const BaseFloat* in_imag , BaseFloat* out_real , BaseFloat* out_imag , uint32 n) const
{
	BaseFloat* conjugated_imag = (BaseFloat*)malloc(sizeof(BaseFloat) * n) ;
	if(NULL == conjugated_imag)  error("no memory to be allocated" , __FILE__ , __LINE__) ;

	for(uint32 i = 0 ; i < n ; ++i)  conjugated_imag[i] = in_imag[i] * (-1.0f) ;

	fft_compute(in_real , conjugated_imag , out_real , out_imag , n) ;

	for(uint32 i = 0 ; i < n ; ++i)
	{
		out_real[i] /= n ; out_imag[i] *= (-1.0f) / n ;
	}

	free(conjugated_imag) ;
}
