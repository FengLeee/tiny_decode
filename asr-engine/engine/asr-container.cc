/*
 * asr-container.cc
 *
 *  Created on : 8/20/2014
 *      Author : zhiming.wang
 */

# include "asr-container.h"

/* initialize */
void ASRContainer::initialize(const struct ASRConfig* asr_config)
{
	acoustic_scale_ = asr_config->acoustic_scale ;
	cmn_window_ = asr_config->cmn_window ;
	decoder_config_.batch_size_ = asr_config->batch_size ;
	decoder_config_.beam_ = asr_config->beam ;
 	decoder_config_.inter_sil_length_ = asr_config->inter_utt_sil ;
	decoder_config_.max_utt_length_ = asr_config->max_utt_length ;
	decoder_config_.min_active_ = asr_config->min_active ;
	decoder_config_.max_active_ = asr_config->max_active ;
	
	filter_bank_options_.mel_options.num_bins = 40 ;

	hmm_ = (HMM*)malloc(sizeof(HMM)) ;
	if(NULL == hmm_)  error("error to allocate memory for hmm" , __FILE__ , __LINE__) ;
	hmm_->initialize() ;
	hmm_->read(asr_config->hmm) ;

	fst_ = (WFST*)malloc(sizeof(WFST)) ;
	if(NULL == fst_)  error("error to allocate memory for fst" , __FILE__ , __LINE__) ;
	fst_->initialize() ;
	fst_->read(asr_config->hclg) ;

	word_symbol_ = (WordSymbol*)malloc(sizeof(WordSymbol)) ;
	if(NULL == word_symbol_)  error("error to allocate memory for word symbol" , __FILE__ , __LINE__) ;
	word_symbol_->initialize() ;
	word_symbol_->read(asr_config->word) ;

	net_interface_ = (NetInterface*)malloc(sizeof(NetInterface)) ;
	if(NULL == net_interface_)  error("error to allocate memory for net interface" , __FILE__ , __LINE__) ;
	net_interface_->initialize(asr_config->feature_transform , asr_config->net_model , asr_config->pdf_count , (1 == asr_config->skip_frame) ? true : false , 0.333333f) ;
	
	fft_ = (FFT*)malloc(sizeof(FFT)) ;
	if(NULL == fft_)  error("error to allocate memory for FFT" , __FILE__ , __LINE__) ;
	fft_->initialize(filter_bank_options_.frame_options.padded_window_size()) ;

	size_t offset = 0 ;
	char ptr_char[64] ;
	for(size_t i = 0 ; i != 1 + strlen(asr_config->sil_phones) ; i++)
	{
		if(':'== asr_config->sil_phones[i] || i == strlen(asr_config->sil_phones))
		{
			strncpy(ptr_char , asr_config->sil_phones + offset , i - offset) ;
			ptr_char[i - offset] = '\0' ;
			sil_phones_.push_back(atoi(ptr_char)) ;
			offset = i + 1 ;
		}
	}
	
	offset = 0 ;
	for(size_t i = 0 ; i != 1 + strlen(asr_config->word_exclusion) ; i++)
	{
		if(':' == asr_config->word_exclusion[i] || i == strlen(asr_config->word_exclusion))
		{
			strncpy(ptr_char , asr_config->word_exclusion + offset , i - offset) ;
			ptr_char[i - offset] = '\0' ;
			word_exclusion_.push_back(atoi(ptr_char)) ;
			offset = i + 1 ;
		}
	}
	assert(0 == word_exclusion_.size() % 2) ;

	if(strlen(asr_config->phone_list) > 0)
	{
		ptr_phone_ = YT_LoadOneTextFileWithKey(0 , (char*)(asr_config->phone_list) , &ptr_phone_buffer_ , &num_phone_entry_) ;
  		assert(NULL != ptr_phone_) ;
	}

	if(strlen(asr_config->word_list) > 0)
	{
		ptr_word_ = YT_LoadOneTextFileWithKey(0 , (char*)(asr_config->word_list) , &ptr_word_buffer_ , &num_word_entry_) ;
  		assert(NULL != ptr_word_) ;
	}
	
	if(strlen(asr_config->syllable) > 0)
	{
		ptr_syllable_ = YT_LoadOneTextFileWithKey(0 , (char*)(asr_config->syllable) , &ptr_syllable_buffer_ , &num_syllable_entry_) ;
  		assert(NULL != ptr_syllable_) ;
	}
	
	if(strlen(asr_config->dict) > 0)
	{
		ptr_dict_ = YT_LoadOneTextFileWithKey(0 , (char*)(asr_config->dict) , &ptr_dict_buffer_ , &num_dict_entry_) ;
  		assert(NULL != ptr_dict_) ;
	}
}

/* release */
void ASRContainer::release()
{
	if(NULL != hmm_)
	{
		hmm_->release() ;
		free(hmm_) ;
		hmm_ = NULL ;
	}

	if(NULL != fst_)
	{
		fst_->release() ;
		free(fst_) ;
		fst_ = NULL ;
	}

	if(NULL != word_symbol_)
	{
		word_symbol_->release() ;
		free(word_symbol_) ;
		word_symbol_ = NULL ;
	}

	if(NULL != net_interface_)
	{
		net_interface_->release() ;
		free(net_interface_) ;
		net_interface_ = NULL ;
	}
    
	if(NULL != fft_)
	{
		fft_->release() ;
		free(fft_) ;
		fft_ = NULL ;
	}

	if(!sil_phones_.empty())	sil_phones_.clear() ;
	
	if(!word_exclusion_.empty())	word_exclusion_.clear() ;

	if(ptr_phone_)
	{
		free(ptr_phone_) ;  ptr_phone_ = NULL ;
	}

	if(ptr_phone_buffer_)
	{
		free(ptr_phone_buffer_) ;  ptr_phone_buffer_ = NULL ;
	}
	
	if(ptr_word_)
	{
		free(ptr_word_) ;  ptr_word_ = NULL ;
	}

	if(ptr_word_buffer_)
	{
		free(ptr_word_buffer_) ;  ptr_word_buffer_ = NULL ;
	}
	
	if(ptr_syllable_)
	{
		free(ptr_syllable_) ;  ptr_syllable_ = NULL ;
	}

	if(ptr_syllable_buffer_)
	{
		free(ptr_syllable_buffer_) ;  ptr_syllable_buffer_ = NULL ;
	}
	
	if(ptr_dict_)
	{
		free(ptr_dict_) ;  ptr_dict_ = NULL ;
	}

	if(ptr_dict_buffer_)
	{
		free(ptr_dict_buffer_) ;  ptr_dict_buffer_ = NULL ;
	}
}
