/*
 * fft.h
 *
 *  Created on : 6/10/2014
 *      Author : zhiming.wang
 */

# ifndef _FFT_H_
# define _FFT_H_

# include "../base/common.h"

class FFT {
public :
	FFT() : butterfly_coefficient_real(NULL) , butterfly_coefficient_imag(NULL) , N(0)  {}
	FFT(uint32 n) ;
	~FFT() ;

	/*  initialize  */
	void initialize(uint32 n) ;

	/*  release  */
	void release() ;

	/*  fast Fourier transform algorithm */
	void fft_compute(const BaseFloat* in_real , const BaseFloat* in_imag , BaseFloat* out_real , BaseFloat* out_imag , uint32 n) const ;

	/*  inverse fast Fourier transform algorithm  */
	void ifft_compute(const BaseFloat* in_real , const BaseFloat* in_imag , BaseFloat* out_real , BaseFloat* out_imag , uint32 n) const ;

private :
	/*  butterfly coefficients sequence , real and  imaginary part respectively  */
	BaseFloat* butterfly_coefficient_real ;
	BaseFloat* butterfly_coefficient_imag ;

	/*  sequence length  */
	uint32 N ;
} ;

# endif /*  _FFT_H_  */
