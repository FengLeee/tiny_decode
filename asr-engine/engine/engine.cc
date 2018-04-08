/*
 * engine.cc
 *
 *  Created on : 08/29/2014
 *      Author : zhiming.wang
 */

# include "engine.h"
//# include <sstream>

ASREngine::~ASREngine()
{
	Release() ;

	mutex_.release() ;
}

void ASREngine::Initialize(const ASRContainer* asr_container)
{
	asr_container_ = asr_container ;

	filter_bank_input_ = (FilterBankInput*)malloc(sizeof(FilterBankInput)) ;
	if(NULL == filter_bank_input_)  error("error to allocate memory for filter bank input" , __FILE__ , __LINE__) ;

	mutex_.initialize() ;
}

void ASREngine::Release()
{
	asr_container_ = NULL ;

	if(NULL != filter_bank_input_)
	{
		filter_bank_input_->release() ;
		free(filter_bank_input_) ;
		filter_bank_input_ = NULL ;
	}

	decoder_.release() ;

	decodable_.release() ;
	
}

void ASREngine::Start()
{
	assert(NULL != asr_container_ || NULL != filter_bank_input_) ;
	
	/* initialize filter bank feature interface */
	filter_bank_input_->initialize(asr_container_->filter_bank_options_ , asr_container_->cmn_window_ , asr_container_->get_fft()) ;
	
	decodable_.initialize(filter_bank_input_ , asr_container_->get_net_interface() , asr_container_->get_hmm() , asr_container_->acoustic_scale_ , asr_container_->decoder_config_.batch_size_) ;
	
	decoder_.initialize(&asr_container_->decoder_config_ , asr_container_->get_fst() , &asr_container_->get_sil_phones() , asr_container_->get_hmm()) ;
}

void ASREngine::Close()
{
	filter_bank_input_->release() ;

	decoder_.release() ;

	decodable_.release() ;

	if(! &string_buffer_)  string_buffer_.clear() ;
	if(! &string_buffer_forward_) string_buffer_forward_.clear() ;

	if(!phone_vec_.empty())  phone_vec_.clear() ;
	if(!phone_stat_.empty())  phone_stat_.clear() ;
}

int32 ASREngine::SendData(int16* data , int32 length)
{
	return filter_bank_input_->get_data_interface()->input_data(data , length) ;
}

int32 ASREngine::StopSendData()
{
	return filter_bank_input_->get_data_interface()->data_finished() ;
}

bool ASREngine::Decode()
{
	DecodeState state = decoder_.decode(&decodable_) ;

	VectorI phone_vec , word_vec ;

	if((state & (EndUtt | EndFeats)) || decoder_.batch_reset())
	{
		decoder_.finish_trace_back(phone_vec , word_vec) ;
		
		RecordPartialResult(phone_vec , word_vec , state == EndUtt) ;
		
		if(state == EndFeats)		  
			return true ;
	} else {
		if(decoder_.local_trace_back(phone_vec , word_vec))
		{	
			RecordPartialResult(phone_vec , word_vec , false) ;
		}
	}

	return false ;
}

void ASREngine::RecordPartialResult(const VectorI& phone_vec , const VectorI& word_vec , bool utt_end)
{
	mutex_.Lock() ;
	
	for(size_t i = 0 ; i != word_vec.size() ; i++)
	{
		
		bool exclusion = false ;
		for(size_t j = 0 ; j != asr_container_->word_exclusion_.size() ; j += 2)
			if(word_vec[i] >= (asr_container_->word_exclusion_)[j] && word_vec[i] <= (asr_container_->word_exclusion_)[j + 1])
			{
				exclusion = true ;
				continue ;
			}
		
		if(!exclusion)
		{	
			
			string_buffer_ += asr_container_->get_word_symbol()->find((size_t)(word_vec[i]));
			//string_buffer_.add_space();
			string_buffer_ += ' ' ;
			
		}
	}
	
	/* if(utt_end)  string_buffer_ += "\n" ;  */
	
	for(size_t i = 0 ; i != phone_vec.size() ; i++)
	{	
		
		if(phone_vec_.empty() || phone_vec_.back() != phone_vec[i])
		{
			phone_vec_.push_back(phone_vec[i]) ;
			phone_stat_.push_back(1) ;
		} else {
			(phone_stat_.back())++ ;
		}
	}

	mutex_.Unlock() ;
}


const char* ASREngine::GetPartialResult() 
{
	
	string_buffer_forward_ = string_buffer_ ;
	
	string_buffer_.clear() ;

	return string_buffer_forward_.c_str();
	
}

/*
const char* ASREngine::GetPartialResult() 
{
	String oss ;

	mutex_.Lock() ;
	
	string_buffer_forward_ = string_buffer_ ;
	
	string_buffer_.clear() ;
	
	/* (phone , continuous phone number) */
/*	for(size_t i = 0 ; i != phone_vec_.size() ; i++){

		oss += "(" ;
		oss +=phone_vec_[i] ;
		oss +=" , " ;
		oss += phone_stat_[i] ;
		oss +=") " ;
	}
	
	phone_vec_.clear() ;

	phone_stat_.clear() ;

	mutex_.Unlock() ;
	
   if(string_buffer_forward_.empty())  

   	return (const char*)("") ;
   else
    {	

	  char* phone_sequence = oss.c_str() ;

	  if(strlen(phone_sequence))
	{
		char phone_out[1500] ;
		char word_out[1500] ;

		YT_ASR_CheckRecResult_NEW_V2(phone_sequence , (char*)(string_buffer_forward_.c_str()) , asr_container_->ptr_syllable_ , asr_container_->num_syllable_entry_ , asr_container_->ptr_dict_ , asr_container_->num_dict_entry_ , asr_container_->ptr_phone_ , asr_container_->num_phone_entry_ ,  \
					        asr_container_->ptr_word_ , asr_container_->num_word_entry_ , phone_out , word_out) ;

		if(strlen((const char*)(word_out)))	string_buffer_forward_ = word_out ;
    }
	}
	

    return string_buffer_forward_.c_str();
	
}
*/
