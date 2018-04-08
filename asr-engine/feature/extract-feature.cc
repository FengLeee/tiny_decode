/*
 * extract-feature.cc
 *
 *  Created on : 6/12/2012
 *      Author : zhiming.wang
 */

# include "extract-feature.h"

uint32 FrameExtractOptions::window_shift() const
{
	return (uint32)(sample_freq * 0.001 * frame_shift_ms) ;
}

uint32 FrameExtractOptions::window_size() const
{
	return (uint32)(sample_freq * 0.001 * frame_length_ms) ;
}

uint32 FrameExtractOptions::padded_window_size() const
{
	return (1 == round_to_power_of_two) ? round_upto_nearest_power_of_two(window_size()) : window_size() ;
}

uint32 FrameExtractOptions::num_frames(uint32 sample_length) const
{
	uint32 frame_shift = window_shift() ;
	uint32 frame_length = window_size() ;
	assert(0 != frame_shift && 0 != frame_length) ;

	return (sample_length < frame_length) ? 0 : (1 + ((sample_length - frame_length) / frame_shift)) ;
}

FeatureWindowFunction::FeatureWindowFunction(const FrameExtractOptions& options) : window(NULL)
{
	initialize(options) ;
}

FeatureWindowFunction::~FeatureWindowFunction()
{
	release() ;
}

/* initialize */
void FeatureWindowFunction::initialize(const FrameExtractOptions& options)
{
	uint32 frame_length = options.window_size() ;
	assert(frame_length > 0) ;

	window = (BaseFloat*)malloc(sizeof(BaseFloat) * frame_length) ;
	if(NULL == window)  error("no memory to be allocated" , __FILE__ , __LINE__) ;

	for(uint32 i = 0 ; i < frame_length ; ++i)
	{
		if(0 == strcmp(options.window_type , "hanning"))
			window[i] = 0.5f - 0.5f * cos(PI2 * (BaseFloat)(i) / (frame_length - 1)) ;
		else if(0 == strcmp(options.window_type , "hamming"))
			window[i] = 0.54f - 0.46f * cos(PI2 * (BaseFloat)(i) / (frame_length - 1)) ;
		else if(0 == strcmp(options.window_type , "kaldi"))  /* like hamming window but go to zero at edges */
			window[i] = pow(0.5f - 0.5f * cos(PI2 * (BaseFloat)(i) / (frame_length - 1)) , 0.85f) ;
		else if(0 == strcmp(options.window_type , "rectangular"))
			window[i] = 1.0f ;
		else
			error("invalid window type" , __FILE__ , __LINE__) ;
	}
}

/* release */
void FeatureWindowFunction::release()
{
	if(NULL != window)
	{
		free(window) ;
		window = NULL ;
	}
}

BaseFloat* FeatureWindowFunction::get_window() const
{
	return window ;
}

/* extract the remainder for further processing */
void extract_waveform_remainder(const BaseFloat* wave , const uint32 length , BaseFloat** wave_remainder , uint32* remainder_length , const FrameExtractOptions& options)
{
	uint32 rows = options.num_frames(length) ;
	uint32 frame_shift = options.window_shift() ;
	uint32 offset = rows * frame_shift ;
	*remainder_length = length - offset ;
	if(*remainder_length > 0)
	{
		assert(NULL != wave_remainder) ;

		if(NULL != *wave_remainder)
		{
			free(*wave_remainder) ;  *wave_remainder = NULL ;
		}

		*wave_remainder = (BaseFloat*)malloc(sizeof(BaseFloat) * (*remainder_length)) ;
		if(NULL == *wave_remainder)  error("no memory to be allocated" , __FILE__ , __LINE__) ;
		memcpy((void*)(*wave_remainder) , (const void*)(wave + offset) , sizeof(BaseFloat) * (*remainder_length)) ;
	}
}

/*
 * extract a windowed frame of waveform with a power-of-two padded size
 * it does mean subtraction , pre-emphasis and dithering as requested
 */
void extract_window(const BaseFloat* wave , const uint32 length , uint32 frame ,
					const FrameExtractOptions& options , const FeatureWindowFunction& window_function ,
					BaseFloat* window , BaseFloat *log_energy_pre_window)
{
	uint32 frame_shift = options.window_shift() ;
	uint32 frame_length = options.window_size() ;
	assert(0 != frame_shift && 0 != frame_length) ;
	uint32 start = frame * frame_shift ;
	assert(start >= 0 && (start + frame_length) <= length) ;
	assert(NULL != window) ;
	uint32 frame_length_padded = options.padded_window_size() ;

	memcpy((void*)window , (const void*)(wave + start) , frame_length * sizeof(BaseFloat)) ;

	if(frame_length < frame_length_padded)  memset((void*)(window + frame_length) , 0 , (frame_length_padded - frame_length) * sizeof(BaseFloat)) ;

	/*
	 * dither
	 * ignored for the time being
	 * if(0.0 != options.dither)  {}
	 */

	/* remove DC , subtract mean */
	if(1 == options.remove_dc_offset)
	{
		BaseFloat sum = 0.0 ;
		for(uint32 i = 0 ; i < frame_length ; ++i)  sum += window[i] ;
		BaseFloat mean = sum / frame_length ;
		for(uint32 i = 0 ; i < frame_length ; ++i)  window[i] -= mean ;
	}

	/* compute log energy */
	if(NULL != log_energy_pre_window)
	{
		BaseFloat squared_sum = 0.0 ;
		for(uint32 i = 0 ; i < frame_length ; ++i)  squared_sum += (window[i] * window[i]) ;
		*log_energy_pre_window = log(squared_sum) ;
	}

	/* pre-emphasize */
	if(0.0 != options.preemph_coeff)
	{
		assert(options.preemph_coeff >= 0.0 && options.preemph_coeff <= 1.0) ;
		for(uint32 i = (frame_length - 1) ; i > 0 ; i--)  window[i] -= window[i - 1] * options.preemph_coeff ;
		window[0] -= window[0] * options.preemph_coeff ;
	}

	BaseFloat* window_coeff = window_function.get_window() ;
	assert(NULL != window_coeff) ;
	for(uint32 i = 0 ; i < frame_length ; ++i)  window[i] *= window_coeff[i] ;
}

/* compute power spectrum */
void compute_power_spectrum(const BaseFloat* real , const BaseFloat* imag , BaseFloat* power_spectrum , uint32 length)
{
	assert(NULL != real) ;
	assert(NULL != imag) ;
	assert(NULL != power_spectrum) ;

	for(uint32 i = 0 ; i < length ; ++i)  power_spectrum[i] = real[i] * real[i] + imag[i] * imag[i] ;
}
