/*
 * compute-filter-bank.cc
 *
 *  Created on : 6/12/2012
 *      Author : zhiming.wang
 */

# include "compute-filter-bank.h"

FilterBank::FilterBank(const FilterBankOptions& filter_bank_options , const FFT* fft) : log_energy_floor_(0.0) , fft_(NULL) , mel_bank_group_(NULL) , mel_bank_group_size_(0)
{
	initialize(filter_bank_options , fft) ;
}

FilterBank::~FilterBank()
{
	release() ;
}

/* initialize */
void FilterBank::initialize(const FilterBankOptions& filter_bank_options , const FFT* fft)
{
	filter_bank_options_ = filter_bank_options ;
	
	feature_window_function_.initialize(filter_bank_options_.frame_options) ;
	
	fft_ = fft ;
	
	if(filter_bank_options_.energy_floor > 0.0)  log_energy_floor_ = log(filter_bank_options_.energy_floor) ;

	mel_bank_group_ = NULL ;
	mel_bank_group_size_ = 0 ;
}

/* release */
void FilterBank::release()
{
	feature_window_function_.release() ;

	fft_ = NULL ;

	if(NULL != mel_bank_group_)
	{
		for(uint32 i = 0 ; i < mel_bank_group_size_ ; ++i)
		{
			if(NULL != mel_bank_group_[i].mel_bank_)
			{
				mel_bank_group_[i].mel_bank_->release() ;
				free(mel_bank_group_[i].mel_bank_) ;
				mel_bank_group_[i].mel_bank_ = NULL ;
			}
			mel_bank_group_[i].vtln_coefficient_ = 0.0 ;
		}
		free(mel_bank_group_) ;
		mel_bank_group_ = NULL ;
	}

	mel_bank_group_size_ = 0 ;
}

/* feature dimension size */
uint32 FilterBank::feature_dim_size() const
{
	return filter_bank_options_.mel_options.num_bins + filter_bank_options_.use_energy ;
}

const MelBank* FilterBank::GetMelBank(BaseFloat vtln_warp)
{
	MelBank* mel_bank = NULL ;
	uint32 i = 0 ;
	for(; i < mel_bank_group_size_ ; i++)
		if(vtln_warp == mel_bank_group_[i].vtln_coefficient_)
		{
			mel_bank = mel_bank_group_[i].mel_bank_ ;
			break ;
		}

	if(NULL == mel_bank)
	{
		i = mel_bank_group_size_ ;
		mel_bank_group_size_++ ;

		if(NULL == mel_bank_group_)  mel_bank_group_ = (MelBankGroup*)malloc(sizeof(MelBankGroup) * mel_bank_group_size_) ;
		else  mel_bank_group_ = (MelBankGroup*)realloc(mel_bank_group_ , sizeof(MelBankGroup) * mel_bank_group_size_) ;

		mel_bank = (MelBank*)malloc(sizeof(MelBank)) ;
		if(NULL == mel_bank)  error("no memory to be allocated" , __FILE__ , __LINE__) ;
		mel_bank->initialize(filter_bank_options_.mel_options , filter_bank_options_.frame_options , vtln_warp) ;

		mel_bank_group_[i].mel_bank_ = mel_bank ;
		mel_bank_group_[i].vtln_coefficient_ = vtln_warp ;
	}

	return mel_bank ;
}

/* compute filter bank */
void FilterBank::compute_filter_bank(const BaseFloat* wave , const uint32 length , BaseFloat vtln_warp ,
							         BaseFloat** output , uint32* num_frame ,
							         BaseFloat** wave_remainder , uint32* remainder_length)
{
	assert(NULL != output) ;

	/* get dimensions of output features */
	uint32 rows = filter_bank_options_.frame_options.num_frames(length) ;
	uint32 cols = feature_dim_size() ;
	if(0 == rows)
	{
		char err[100] ;
		sprintf(err , "no frames left , and samples number is %u" , length) ;
		error(err , __FILE__ , __LINE__) ;
	}

	if(NULL == *output)
	{
		*output = (BaseFloat*)malloc(sizeof(BaseFloat) * rows * cols) ;
		if(NULL == *output)  error("no memory to be allocated" , __FILE__ , __LINE__) ;
	}

	*num_frame = rows ;

	/* optionally extract the remainder for further processing */
	if(NULL != wave_remainder)  extract_waveform_remainder(wave , length , wave_remainder , remainder_length , filter_bank_options_.frame_options) ;

	/* buffer */
	BaseFloat log_energy ;
	uint32 frame_length_padded = filter_bank_options_.frame_options.padded_window_size() ;
	BaseFloat* window = (BaseFloat*)malloc(sizeof(BaseFloat) * frame_length_padded) ;
	BaseFloat* fft_real = (BaseFloat*)malloc(sizeof(BaseFloat) * frame_length_padded) ;
	BaseFloat* fft_imag = (BaseFloat*)malloc(sizeof(BaseFloat) * frame_length_padded) ;
	BaseFloat* power_spectrum = (BaseFloat*)malloc(sizeof(BaseFloat) * (1 + (frame_length_padded >> 1))) ;
	BaseFloat* mel_energies = (BaseFloat*)malloc(sizeof(BaseFloat) * filter_bank_options_.mel_options.num_bins) ;
	if(NULL == window || NULL == fft_real || NULL == fft_imag || NULL == power_spectrum || NULL == mel_energies)
		error("no memory to be allocated" , __FILE__ , __LINE__) ;

	/* compute all the frames , r is frame index */
	for(uint32 r = 0 ; r < rows ; ++r)
	{
		/* extract a windowed frame */
		extract_window(wave , length , r , filter_bank_options_.frame_options , feature_window_function_ ,  \
				       window , ((1 == filter_bank_options_.use_energy) && (1 == filter_bank_options_.raw_energy)) ? &log_energy : NULL) ;

		/* compute energy after window function (not the raw one)  */
		if((1 == filter_bank_options_.use_energy) && (0 == filter_bank_options_.raw_energy))
		{
			BaseFloat squared_sum = 0.0 ;
			for(uint32 i = 0 ; i < frame_length_padded ; ++i)  squared_sum += (window[i] * window[i]) ;
			log_energy = log(squared_sum) ;
		}

		/* compute FFT */
		fft_->fft_compute(window , NULL , fft_real , fft_imag , frame_length_padded) ;

		/* convert the FFT into a power spectrum */
		compute_power_spectrum(fft_real , fft_imag , power_spectrum , 1 + (frame_length_padded >> 1)) ;

		const MelBank* mel_bank = GetMelBank(vtln_warp) ;
		mel_bank->compute_mel_bank(power_spectrum , mel_energies) ;

		if(1 == filter_bank_options_.use_log_fbank)
			for(uint32 j = 0 ; j < filter_bank_options_.mel_options.num_bins ; ++j)
				mel_energies[j] = log(mel_energies[j]) ;

		/* output buffer */
		BaseFloat* this_output = (*output) + r * cols ;

		if(1 == filter_bank_options_.use_energy)
		{
			memcpy((void*)(1 + this_output) , (const void*)mel_energies , sizeof(BaseFloat) * filter_bank_options_.mel_options.num_bins) ;

			if(filter_bank_options_.energy_floor > 0.0 && log_energy < log_energy_floor_)  log_energy = log_energy_floor_ ;
			*this_output = log_energy ;
		} else {
			memcpy((void*)(this_output) , (const void*)mel_energies , sizeof(BaseFloat) * filter_bank_options_.mel_options.num_bins) ;
		}
	}

	free(window) ;
	free(fft_real) ;
	free(fft_imag) ;
	free(power_spectrum) ;
	free(mel_energies) ;
 }
