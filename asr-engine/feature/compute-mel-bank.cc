/*
 * compute-mel-bank.cc
 *
 *  Created on : 6/13/2014
 *      Author : zhiming.wang
 */

# include "compute-mel-bank.h"

MelBank::MelBank(const MelBankOptions& mel_options , const FrameExtractOptions& frame_options , BaseFloat vtln_warp_factor)
: num_bins_(0) , vtln_warp_factor_(-1.0) , bins_offset_(NULL) , bins_weight_size_(NULL) , bins_weight_(NULL)
{
	initialize(mel_options , frame_options , vtln_warp_factor) ;
}

MelBank::~MelBank()
{
	release() ;
}

/* initialize */
void MelBank::initialize(const MelBankOptions& mel_options , const FrameExtractOptions& frame_options , BaseFloat vtln_warp_factor)
{
	num_bins_ = mel_options.num_bins ;
	assert(num_bins_ >= 3) ;

	vtln_warp_factor_ = vtln_warp_factor ;

	BaseFloat sample_freq = frame_options.sample_freq ;
	uint32 window_length_padded = frame_options.padded_window_size() ;

	uint32 num_fft_bins = window_length_padded >> 1 ;
	BaseFloat nyquist = 0.5f * sample_freq ;

	BaseFloat low_freq = mel_options.low_freq , high_freq ;
	if(mel_options.high_freq > 0.0)   high_freq = mel_options.high_freq ;
	else  high_freq = nyquist + mel_options.high_freq ;
	if(low_freq < 0.0 || low_freq >= sample_freq || high_freq <= 0.0 || high_freq > sample_freq || high_freq <= low_freq)
	{
		char err[100] ;
		sprintf(err , "bad values with options low_freq %f and high_freq %f" , low_freq  , high_freq) ;
		error(err , __FILE__ , __LINE__) ;
	}

	BaseFloat fft_bin_width = sample_freq / window_length_padded ;  /* fft-bin-width [think of it as Nyquist-freq / half-window-length] */

	BaseFloat mel_low_freq = mel_scale(low_freq) ;
	BaseFloat mel_high_freq = mel_scale(high_freq) ;

	/* divide by num_bins + 1 in next line because of end-effects where the bins spread out to the sides */
	BaseFloat mel_freq_delta = (mel_high_freq - mel_low_freq) / (num_bins_ + 1) ;

	BaseFloat vtln_low = mel_options.vtln_low , vtln_high = mel_options.vtln_high ;
	if(vtln_high < 0.0)    vtln_high += nyquist ;

	if(1.0 != vtln_warp_factor &&
       (vtln_low < 0.0 || vtln_low <= low_freq
       || vtln_low >= high_freq
       || vtln_high <= 0.0 || vtln_high >= high_freq
       || vtln_high <= vtln_low))
	{
		char err[100] ;
		sprintf(err , "bad values with options vtln_low %f , vtln_high %f , low_freq %f and high_freq %f" , vtln_low , vtln_high , low_freq , high_freq) ;
		error(err , __FILE__ , __LINE__) ;
	}

	bins_offset_ = (uint32*)malloc(num_bins_ * sizeof(uint32)) ;
	bins_weight_size_ = (uint32*)malloc(num_bins_ * sizeof(uint32)) ;
	bins_weight_ = (BaseFloat**)malloc(num_bins_ * sizeof(BaseFloat*)) ;
	BaseFloat* bins_weight = (BaseFloat*)malloc(num_fft_bins * sizeof(BaseFloat)) ;
	if(NULL == bins_offset_ || NULL == bins_weight_size_ || NULL == bins_weight_ || NULL == bins_weight)
		error("no memory to be allocated" , __FILE__ , __LINE__) ;

	for(uint32 bin = 0 ; bin < num_bins_ ; bin++)
	{
		BaseFloat left_mel = mel_low_freq + bin * mel_freq_delta ,
				  center_mel = mel_low_freq + (bin + 1) * mel_freq_delta ,
                  right_mel = mel_low_freq + (bin + 2) * mel_freq_delta ;

		if(1.0 != vtln_warp_factor)
		{
		    left_mel = vtln_warp_mel_freq(vtln_low , vtln_high , low_freq , high_freq , vtln_warp_factor , left_mel) ;
		    center_mel = vtln_warp_mel_freq(vtln_low , vtln_high , low_freq , high_freq , vtln_warp_factor , center_mel) ;
		    right_mel = vtln_warp_mel_freq(vtln_low , vtln_high , low_freq , high_freq , vtln_warp_factor , right_mel) ;
	    }

	    int32 first_index = -1 , last_index = -1 ;
	    memset((void*)bins_weight , 0 , num_fft_bins * sizeof(BaseFloat)) ;

	    for(uint32 i = 0 ; i < num_fft_bins ; i++)
	    {
		    BaseFloat freq = (fft_bin_width * i) ;  /* center freq of this fft bin */
		    BaseFloat mel = mel_scale(freq) ;

		    if(mel > left_mel && mel <= center_mel)
		    {
			    bins_weight[i] = (mel - left_mel) / (center_mel - left_mel) ;
		        if(-1 == first_index)  first_index = ((int32)i) ;
		        last_index = ((int32)i) ;  /* this statement only needed to handle very short bins that arise in pathological cases */
		    }else if(mel > center_mel && mel < right_mel)
		    {
			    bins_weight[i] = (right_mel - mel) / (right_mel - center_mel) ;
		        last_index = ((int32)i) ;
		    }
	    }

	    assert(-1 != first_index && last_index >= first_index) ;
	    bins_offset_[bin] = (uint32)first_index ;
	    bins_weight_size_[bin] = (uint32)(last_index + 1 - first_index) ;
	    bins_weight_[bin] = (BaseFloat*)malloc(bins_weight_size_[bin] * sizeof(BaseFloat)) ;
	    if(NULL == bins_weight_[bin])  error("no memory to be allocated" , __FILE__ , __LINE__) ;
	    memcpy((void*)(bins_weight_[bin]) , (const void*)(bins_weight + bins_offset_[bin]) , sizeof(BaseFloat) * (bins_weight_size_[bin])) ;
	}

	free(bins_weight) ;
}

/* release */
void MelBank::release()
{
	if(NULL != bins_offset_)
	{
		free(bins_offset_) ;
		bins_offset_ = NULL ;
	}

	if(NULL != bins_weight_)
	{
		for(uint32 i = 0 ; i < num_bins_ ; ++i)
		{
			if(NULL != bins_weight_[i])
			{
				free(bins_weight_[i]) ;
				bins_weight_[i] = NULL ;
			}
		}
		free(bins_weight_) ;
		bins_weight_ = NULL ;
	}

	if(NULL != bins_weight_size_)
	{
		free(bins_weight_size_) ;
		bins_weight_size_ = NULL ;
	}

	num_bins_ = 0 ;
}

/* "power_spectrum" contains fft energies */
void MelBank::compute_mel_bank(const BaseFloat* power_spectrum , BaseFloat* mel_energies) const
{
	assert(NULL != power_spectrum && NULL != mel_energies) ;

	for(uint32 bin = 0 ; bin < num_bins_ ; ++bin)
	{
		BaseFloat energy = 0.0 ;
		uint32 offset = bins_offset_[bin] ;
		for(uint32 i = 0 ; i < bins_weight_size_[bin] ; ++i)
			energy += bins_weight_[bin][i] * power_spectrum[offset + i] ;

		assert(!ISNAN(energy)) ;
		mel_energies[bin] = energy ;
	}
}

/* frequency -> mel-frequency */
BaseFloat MelBank::mel_scale(BaseFloat freq)
{
	return 1127.0f * log(1.0f + freq / 700.0f) ;
}

/* mel-frequency -> frequency */
BaseFloat MelBank::inverse_mel_scale(BaseFloat mel_freq)
{
	return 700.0f * (exp(mel_freq / 1127.0f) - 1.0f) ;
}

/*
 * VTLN --- Vocal Tract Length Normalization
 * this function computes a warp function F(freq) , defined between low_freq and
 * high_freq inclusive , with the following properties :
 * F(low_freq) == low_freq
 * F(high_freq) == high_freq
 * the function is continuous and piecewise linear with two inflection points .
 * the lower inflection point (measured in terms of the unwarped
 * frequency) is at frequency l , determined as described below .
 * the higher inflection point is at a frequency h , determined as
 * described below .
 * if l <= f <= h , then F(f) = f/vtln_warp_factor .
 * if the higher inflection point (measured in terms of the unwarped
 * frequency) is at h , then max(h , F(h)) == vtln_high_cutoff .
 * since (by the last point) F(h) == h/vtln_warp_factor , then
 * max(h , h / vtln_warp_factor) == vtln_high_cutoff , so
 * h = vtln_high_cutoff / max(1 , 1 / vtln_warp_factor)
 *   = vtln_high_cutoff * min(1 , vtln_warp_factor) .
 * if the lower inflection point (measured in terms of the unwarped
 * frequency) is at l , then min(l , F(l)) == vtln_low_cutoff
 * this implies that l = vtln_low_cutoff / min(1 , 1 / vtln_warp_factor)
 *                     = vtln_low_cutoff * max(1 , vtln_warp_factor)
 */
BaseFloat MelBank::vtln_warp_freq(BaseFloat vtln_low_cutoff ,  /* upper and lower frequency cutoffs for VTLN */
                                  BaseFloat vtln_high_cutoff ,
                                  BaseFloat low_freq ,         /* upper and lower frequency cutoffs in mel computation */
                                  BaseFloat high_freq ,
                                  BaseFloat vtln_warp_factor ,
                                  BaseFloat freq)
{
	 if (freq < low_freq || freq > high_freq)  return freq ;  /* in case this gets called for out-of-range frequencies , just return the freq */

	 assert(vtln_low_cutoff > low_freq) ;
	 assert(vtln_high_cutoff < high_freq) ;

	 BaseFloat one = 1.0f ;
	 BaseFloat l = vtln_low_cutoff * MAX(one , vtln_warp_factor) ;
	 BaseFloat h = vtln_high_cutoff * MIN(one , vtln_warp_factor) ;
	 BaseFloat scale = 1.0f / vtln_warp_factor ;
	 BaseFloat Fl = scale * l ;  /* F(l) */
	 BaseFloat Fh = scale * h ;  /* F(h) */
	 assert(l > low_freq && h < high_freq) ;

	 /* slope of left part of the 3-piece linear function */
	 BaseFloat scale_left = (Fl - low_freq) / (l - low_freq) ;

	 /* [slope of center part is just "scale"] */

	 /* slope of right part of the 3-piece linear function */
	 BaseFloat scale_right = (high_freq - Fh) / (high_freq - h) ;

	 if (freq < l)         return low_freq + scale_left * (freq - low_freq) ;
	 else if (freq < h)    return scale * freq ;
	 else                  return high_freq + scale_right * (freq - high_freq) ;    /* freq >= h */
}

BaseFloat MelBank::vtln_warp_mel_freq(BaseFloat vtln_low_cutoff ,    /* upper and lower frequency cutoffs for VTLN */
                                      BaseFloat vtln_high_cutoff ,
                                      BaseFloat low_freq ,           /* upper and lower frequency cutoffs in mel computation */
                                      BaseFloat high_freq ,
                                      BaseFloat vtln_warp_factor ,
                                      BaseFloat mel_freq)
{
  return mel_scale(vtln_warp_freq(vtln_low_cutoff , vtln_high_cutoff ,
                               	  low_freq , high_freq ,
                               	  vtln_warp_factor , inverse_mel_scale(mel_freq))) ;
}
