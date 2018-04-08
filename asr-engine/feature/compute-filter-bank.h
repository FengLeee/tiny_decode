/*
 * compute-filter-bank.h
 *
 *  Created on : 6/12/2012
 *      Author : zhiming.wang
 */

# ifndef _COMPUTE_FILTER_BANK_H_
# define _COMPUTE_FILTER_BANK_H_

# include "extract-feature.h"
# include "compute-mel-bank.h"
# include "../math/fft.h"

struct FilterBankOptions {
	struct FrameExtractOptions frame_options ;
	struct MelBankOptions mel_options ;
	uint32 use_energy ;                            /* use energy */
	uint32 raw_energy ;                            /* compute energy before pre-emphasis and hamming window (else after) */
	uint32 use_log_fbank ;                         /* if true (default) , produce log filter bank , else linear */
	BaseFloat energy_floor ;

	FilterBankOptions() : use_energy(0) , raw_energy(1) , use_log_fbank(1) , energy_floor(0.0)  {}

	inline void initialize()
	{
		frame_options.initialize() ;
		mel_options.initialize() ;
		use_energy = 0 ;
		raw_energy = 1 ;
		use_log_fbank = 1 ;
		energy_floor = 0.0 ;
	}

} ;

struct MelBankGroup {
	BaseFloat vtln_coefficient_ ;
	MelBank* mel_bank_ ;

	MelBankGroup() : vtln_coefficient_(0.0) , mel_bank_(NULL) {}

} ;

class FilterBank {
public :
	FilterBank() : log_energy_floor_(0.0) , fft_(NULL) , mel_bank_group_(NULL) , mel_bank_group_size_(0) {}
	FilterBank(const FilterBankOptions& filter_bank_options , const FFT* fft) ;
	~FilterBank() ;

	/* initialize */
	void initialize(const FilterBankOptions& filter_bank_options , const FFT* fft) ;

	/* release */
	void release() ;

	/* feature dimension size */
	uint32 feature_dim_size() const ;

	/* compute filter bank */
	void compute_filter_bank(const BaseFloat* wave , const uint32 length , BaseFloat vtln_warp ,
							 BaseFloat** output , uint32* num_frame ,
							 BaseFloat** wave_remainder , uint32* remainder_length) ;

private :
	const MelBank* GetMelBank(BaseFloat vtln_warp) ;

	FilterBankOptions filter_bank_options_ ;
	BaseFloat log_energy_floor_ ;
	FeatureWindowFunction feature_window_function_ ;
	const FFT* fft_ ;
	MelBankGroup* mel_bank_group_ ;
	uint32 mel_bank_group_size_ ;

} ;

# endif /* _COMPUTE_FILTER_BANK_H_ */
