/*
 * feature-input.h
 *
 *  Created on : 07/29/2014
 *      Author : zhiming.wang
 */

# ifndef _FEATURE_INPUT_H_
# define _FEATURE_INPUT_H_

# include "../util/data-interface.h"
# include "../feature/compute-filter-bank.h"
# include "../feature/cmvn.h"

class FilterBankInput
{
public :
	FilterBankInput() ;
	FilterBankInput(const FilterBankOptions& filter_bank_options , uint32 window_size , const FFT* fft) ;
	~FilterBankInput() ;

	/* initialize */
	void initialize(const FilterBankOptions& filter_bank_options , uint32 window_size , const FFT* fft) ;

	/* release */
	void release() ;

	/*
	 *  1 . grab raw (audio) data ;
	 *  2 . extract filter bank features ;
	 *  3 . apply CMN
	 */
	bool compute(BaseFloat** output , uint32* num_row , uint32* num_col) ;

	uint32 get_feature_input_dim() const ;
	
	DateInterface* get_data_interface() const
	{
		return data_interface_ ;
	}

private :
	DateInterface* data_interface_ ;
	FilterBank* filter_bank_ ;
	CMN* cmn_ ;

	uint32 feature_dim_size_ ;
	uint32 frame_size_ ;
	uint32 frame_shift_ ;

	BaseFloat* wave_remainder_ ;
	uint32 remainder_length_ ;

} ;

# endif /* _FEATURE_INPUT_H_ */
