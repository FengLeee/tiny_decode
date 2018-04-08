/*
 * feature-input.cc
 *
 *  Created on : 07/29/2014
 *      Author : zhiming.wang
 */

# include "decode/feature-input.h"

FilterBankInput::FilterBankInput() : data_interface_(NULL) , filter_bank_(NULL) , cmn_(NULL) , feature_dim_size_(0) , frame_size_(0) , frame_shift_(0) , wave_remainder_(NULL) , remainder_length_(0)
{
}

FilterBankInput::FilterBankInput(const FilterBankOptions& filter_bank_options , uint32 window_size , const FFT* fft) : data_interface_(NULL) , filter_bank_(NULL) , cmn_(NULL) , feature_dim_size_(0) , frame_size_(0) , frame_shift_(0) , wave_remainder_(NULL) , remainder_length_(0)
{
	initialize(filter_bank_options , window_size , fft) ;
}

FilterBankInput::~FilterBankInput()
{
	release() ;
}

/* initialize */
void FilterBankInput::initialize(const FilterBankOptions& filter_bank_options , uint32 window_size , const FFT* fft)
{
	data_interface_ = (DateInterface*)malloc(sizeof(DateInterface)) ;
	if(NULL == data_interface_) error("no memory to be allocated" , __FILE__ , __LINE__) ;
	data_interface_->initialize() ;

	filter_bank_ = (FilterBank*)malloc((sizeof(FilterBank))) ;
	if(NULL == filter_bank_) error("no memory to be allocated" , __FILE__ , __LINE__) ;
	filter_bank_->initialize(filter_bank_options , fft) ;
	feature_dim_size_ = filter_bank_->feature_dim_size() ;
	
	cmn_ = (CMN*)malloc(sizeof(CMN)) ;
	if(NULL == cmn_) error("no memory to be allocated" , __FILE__ , __LINE__) ;
	cmn_->initialize(feature_dim_size_ , window_size) ;

	frame_size_ = filter_bank_options.frame_options.window_size() ;
	frame_shift_ = filter_bank_options.frame_options.window_shift() ;

	wave_remainder_ = NULL ;
	remainder_length_ = 0 ;
}

/* release */
void FilterBankInput::release()
{
	if(NULL != data_interface_)
	{
		data_interface_->release() ;
		free(data_interface_) ;
		data_interface_ = NULL ;
	}

	if(NULL != filter_bank_)
	{
		filter_bank_->release() ;
		free(filter_bank_) ;
		filter_bank_ = NULL ;
	}

	if(NULL != cmn_)
	{
		cmn_->release() ;
		free(cmn_) ;
		cmn_ = NULL ;
	}

	Free(&wave_remainder_) ;
}

/*
 *  1 . grab raw (audio) data ;
 *  2 . extract filter bank features ;
 *  3 . apply CMN
 */
bool FilterBankInput::compute(BaseFloat** output , uint32* num_row , uint32* num_col)
{
	assert(*num_col == feature_dim_size_) ;

	if(*num_row <= 0)
	{
		printf("warn : no feature vectors requested") ;
		return true ;
	}


	uint32 request_length = frame_size_ + (*num_row - 1) * frame_shift_ ;

	uint32 wave_length = request_length ;


	/* grab raw (audio) data */
    BaseFloat* wave = NULL ;
    Malloc(&wave , sizeof(BaseFloat) * request_length) ;
    BaseFloat* ptr_wave = wave ;

    if(remainder_length_ && wave_remainder_)
    {
    	memcpy((void*)wave , (const void*)wave_remainder_ , sizeof(BaseFloat) * remainder_length_) ;
    	request_length -= remainder_length_ ;
    	ptr_wave += remainder_length_ ;
    }

    uint32 acquire_length = (uint32)(data_interface_->read(ptr_wave , request_length)) ;

    if(acquire_length != request_length)
    {
    	wave_length = remainder_length_ + acquire_length ;

    	if(wave_length < frame_size_)
    	{
    		*num_row = 1 ;

    		for(uint32 i = wave_length ; i < frame_size_ ; ++i)  wave[i] = 0.0 ;

    		wave_length = frame_size_ ;
    	}
    }

    /* extract filter bank features */
    filter_bank_->compute_filter_bank(wave , wave_length , 1.0 , output , num_row , &wave_remainder_ , &remainder_length_) ;

    /* apply CMN */
    cmn_->apply_cmn(*output , *num_row , *num_col) ;

    Free(&wave) ;

	return request_length == acquire_length ;
}

uint32 FilterBankInput::get_feature_input_dim() const
{
	return feature_dim_size_ ;
}