/*
 * compute-mel-bank.h
 *
 *  Created on : 6/13/2014
 *      Author : zhiming.wang
 */

# ifndef _COMPUTE_MEL_BANK_H_
# define _COMPUTE_MEL_BANK_H_

# include "extract-feature.h"

struct MelBankOptions
{
	uint32 num_bins ;            /* e.g. 40 , number of triangular bins */
	BaseFloat low_freq ;         /* e.g. 20 , lower frequency cutoff */
	BaseFloat high_freq ;        /* an upper frequency cutoff ; 0 -> no cutoff , negative -> added to the Nyquist frequency to get the cutoff */
	BaseFloat vtln_low ;         /* vtln lower cutoff of warping function */
	BaseFloat vtln_high ;        /* vtln upper cutoff of warping function , if negative , added to the Nyquist frequency to get the cutoff */

	MelBankOptions() : num_bins(40) , low_freq(20.0) , high_freq(0.0) , vtln_low(100.0) , vtln_high(-500.0)
	{}
	
	inline void initialize()
	{
		num_bins = 40 ;
		low_freq = 20.0 ;
		high_freq = 0.0 ;
		vtln_low = 100.0 ;
		vtln_high = -500.0 ;
	}

} ;

class MelBank {
public :
	MelBank() : num_bins_(0) , vtln_warp_factor_(-1.0) , bins_offset_(NULL) , bins_weight_size_(NULL) , bins_weight_(NULL) {}
	MelBank(const MelBankOptions& mel_options , const FrameExtractOptions& frame_options , BaseFloat vtln_warp_factor) ;
	~MelBank() ;

	/* initialize */
	void initialize(const MelBankOptions& mel_options , const FrameExtractOptions& frame_options , BaseFloat vtln_warp_factor) ;

	/* release */
	void release() ;

	void compute_mel_bank(const BaseFloat* power_spectrum , BaseFloat* mel_energies) const;

private :
	/* frequency -> mel-frequency */
	BaseFloat mel_scale(BaseFloat freq) ;

	/* mel-frequency -> frequency */
	BaseFloat inverse_mel_scale(BaseFloat mel_freq) ;

	BaseFloat vtln_warp_freq(BaseFloat vtln_low_cutoff ,  /* upper and lower frequency cutoffs for VTLN */
                             BaseFloat vtln_high_cutoff ,
                             BaseFloat low_freq ,         /* upper and lower frequency cutoffs in mel computation */
                             BaseFloat high_freq ,
                             BaseFloat vtln_warp_factor ,
                             BaseFloat freq) ;

	BaseFloat vtln_warp_mel_freq(BaseFloat vtln_low_cutoff ,    /* upper and lower frequency cutoffs for VTLN */
                                 BaseFloat vtln_high_cutoff ,
                                 BaseFloat low_freq ,           /* upper and lower frequency cutoffs in mel computation */
                                 BaseFloat high_freq ,
                                 BaseFloat vtln_warp_factor ,
                                 BaseFloat mel_freq) ;

	uint32 num_bins_ ;
	BaseFloat vtln_warp_factor_ ;
	uint32* bins_offset_ ;
	uint32* bins_weight_size_ ;
	BaseFloat** bins_weight_ ;

} ;

# endif /* _COMPUTE_MEL_BANK_H_ */
