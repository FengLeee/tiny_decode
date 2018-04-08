/*
 * extract-feature.h
 *
 *  Created on : 6/12/2012
 *      Author : zhiming.wang
 */

# ifndef _EXTRACT_FEATURE_H_
# define _EXTRACT_FEATURE_H_

# include "../base/common.h"

struct FrameExtractOptions {
	BaseFloat sample_freq ;
	BaseFloat frame_shift_ms ;                 /* in milliseconds */
	BaseFloat frame_length_ms ;                /* in milliseconds */
	BaseFloat dither ;                         /* amount of dithering , 0.0 means no dither */
	BaseFloat preemph_coeff ;  				   /* pre-emphasis coefficient */
	int32 remove_dc_offset ;                   /* subtract mean of wave before FFT */
	const char* window_type ;                  /* e.g. Hamming window */
	int32 round_to_power_of_two ;

	FrameExtractOptions() : sample_freq(16000.0f) , frame_shift_ms(10.0f) , frame_length_ms(25.0f) , dither(0.0f) , preemph_coeff(0.97f) , remove_dc_offset(1) , window_type("kaldi") , round_to_power_of_two(1)
	{}
	
	inline void initialize()
	{
		sample_freq = 16000.0f ;
		frame_shift_ms = 10.0f ;
		frame_length_ms = 25.0f ;
		dither = 0.0f ;
		preemph_coeff = 0.97f ;
		remove_dc_offset = 1 ;
		window_type = "kaldi" ;
		round_to_power_of_two = 1 ;
	}
	
	uint32 window_shift() const ;

	uint32 window_size() const ;

	uint32 padded_window_size() const ;

	uint32 num_frames(uint32 sample_length) const ;

} ;

class FeatureWindowFunction {
public :
	FeatureWindowFunction() : window(NULL) {}
	FeatureWindowFunction(const FrameExtractOptions& options) ;
	~FeatureWindowFunction() ;

	/* initialize */
	void initialize(const FrameExtractOptions& options) ;

	/* release */
	void release() ;

	BaseFloat* get_window() const ;

private :
	BaseFloat* window ;

} ;

/* extract the remainder for further processing */
void extract_waveform_remainder(const BaseFloat* wave , const uint32 length , BaseFloat** wave_remainder , uint32* remainder_length , const FrameExtractOptions& options) ;

/*
 * extract a windowed frame of waveform with a power-of-two padded size
 * it does mean subtraction , pre-emphasis and dithering as requested
 */
void extract_window(const BaseFloat* wave , const uint32 length , uint32 frame ,
					const FrameExtractOptions& options , const FeatureWindowFunction& window_function ,
					BaseFloat* window , BaseFloat *log_energy_pre_window) ;

/* compute power spectrum */
void compute_power_spectrum(const BaseFloat* real , const BaseFloat* imag , BaseFloat* power_spectrum , uint32 length) ;

# endif /* _EXTRACT_FEATURE_H_ */
