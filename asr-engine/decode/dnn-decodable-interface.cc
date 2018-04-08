/*
 * dnn-decodable-interface.cc
 *
 *  Created on : 07/30/2014
 *      Author : zhiming.wang
 */

# include "decode/dnn-decodable-interface.h"

DNNDecodableInterface::DNNDecodableInterface() : log_likelihood_(NULL) , filter_bank_input_(NULL) , net_interface_(NULL) , hmm_(NULL) , acoustic_scale_(1.0) , batch_size_(0) , batch_size_inc_(0) , num_row_(0) , feature_input_dim_(0) , feature_output_dim_(0) , feature_offset_(0) , finished_(false) , skip_frame(false) , \
net_feature_buffer_(NULL) , feature_row_offset_(0) , covered_feature_row_offset_(0) , buffer_size_(0) , buffer_size_inc_(0) 
{
}

DNNDecodableInterface::DNNDecodableInterface(FilterBankInput* filter_bank_input , const NetInterface* net_interface , const HMM* hmm , BaseFloat acoustic_scale , uint32 batch_size) : log_likelihood_(NULL) , filter_bank_input_(NULL) , net_interface_(NULL) , hmm_(NULL) , acoustic_scale_(1.0) , batch_size_(0) , batch_size_inc_(0) , num_row_(0) , feature_input_dim_(0) , feature_output_dim_(0) , feature_offset_(0) , finished_(false) , skip_frame(false) ,  \
net_feature_buffer_(NULL) , feature_row_offset_(0) , covered_feature_row_offset_(0) , buffer_size_(0) , buffer_size_inc_(0) 
{
	initialize(filter_bank_input , net_interface , hmm , acoustic_scale , batch_size) ;
}

DNNDecodableInterface::~DNNDecodableInterface()
{
	release() ;
}

/* initialize */
void DNNDecodableInterface::initialize(FilterBankInput* filter_bank_input , const NetInterface* net_interface , const HMM* hmm , BaseFloat acoustic_scale ,  uint32 batch_size)
{
	assert(NULL != filter_bank_input) ;
	filter_bank_input_ = filter_bank_input ;

	assert(NULL != net_interface) ;
	net_interface_ = net_interface ;

	assert(NULL != hmm) ;
	hmm_ = hmm ;

	acoustic_scale_ = acoustic_scale ;

	batch_size_ = batch_size ;
	batch_size_inc_ = 0 ;

	num_row_ = 0 ;

	feature_output_dim_ = (uint32)((const NetInterface*)net_interface_)->output_dim() ;
	assert(((const HMM*)hmm_)->num_pdfs() == feature_output_dim_) ;
	
	feature_input_dim_ = (uint32)((const NetInterface*)net_interface_)->input_dim() ;

	feature_offset_ = 0 ;

	finished_ = false ;

	/* is to skip every two frame ? */
	skip_frame = ((const NetInterface*)net_interface_)->get_net_input()->skipping_frame() ;
	
	Malloc(&log_likelihood_ , (size_t)(sizeof(BaseFloat) * batch_size_ * feature_output_dim_ * pow(3 , batch_size_inc_limit))) ;

	buffer_size_ = buffer_size_inc_ =  feature_input_dim_ * 1000 ;

	Malloc(&net_feature_buffer_ , sizeof(BaseFloat) *  buffer_size_) ;

	feature_row_offset_ = 0 ;
	covered_feature_row_offset_ = 0 ;
}

/* release */
void DNNDecodableInterface::release()
{
	Free(&log_likelihood_) ;
	
	if(NULL != net_feature_buffer_)  Free(&net_feature_buffer_) ;

	buffer_size_ = buffer_size_inc_ = feature_row_offset_ = covered_feature_row_offset_ = 0 ;
}

/* index are one-based , for compatibility with OpenFst */
uint32 DNNDecodableInterface::num_transitions()  const
{
	return ((const HMM*)hmm_)->num_transitions() ;
}

/* index means transition id */
BaseFloat DNNDecodableInterface::log_likelihood(int32 frame , int32 index)
{
	uint32 corresponding_frame = (uint32)(skip_frame ? (frame >> 1) : frame) ;

	if(corresponding_frame >= feature_offset_ + num_row_)
	{
		assert(false == finished_) ;

		feature_offset_ += num_row_ ;

		/* initialize the underlying decodable interface with a new batch of features  */
		int32 right_context = ((const NetInterface*)net_interface_)->get_net_input()->right_context();
		BaseFloat* feature_input = NULL ;
		uint32 num_row = batch_size_ + right_context ;
		uint32 num_col = feature_input_dim_ ;

		finished_ = !filter_bank_input_->compute(&feature_input , &num_row , &num_col) ;
		
		if((feature_row_offset_ + num_row) * feature_input_dim_ > (uint32)(buffer_size_))
		{
			buffer_size_ += buffer_size_inc_ ;
			BaseFloat* net_feature_buffer = (BaseFloat*)realloc(net_feature_buffer_ , sizeof(BaseFloat) * buffer_size_) ;
			if(NULL !=  net_feature_buffer)  net_feature_buffer_ =  net_feature_buffer ;
			else  error("Error : could not re-allocate memory for net feature buffer" , __FILE__ , __LINE__) ;
		}
		memcpy((void*)(net_feature_buffer_ + feature_row_offset_ * feature_input_dim_) , (const void*)feature_input , sizeof(BaseFloat) * num_row * num_col) ;
		feature_row_offset_ += num_row ;
		
		uint32 covered_row_size = feature_row_offset_ - covered_feature_row_offset_ - (finished_ ? 0 : right_context) ;
		num_row_  = (false == skip_frame) ? covered_row_size : ((0 == (covered_row_size % 2)) ? (covered_row_size >> 1) : (1 + (covered_row_size >> 1))) ;
		
		/* compute net forward */
		/* consider acoustic scale , too */
		((const NetInterface*)net_interface_)->compute(net_feature_buffer_ , (int32)num_row , (int32)num_col , log_likelihood_ , (int32)num_row_ , (int32)feature_output_dim_ , acoustic_scale_ , feature_row_offset_ , covered_feature_row_offset_ , covered_row_size) ;

		Free(&feature_input) ;

		if((++batch_size_inc_) <= batch_size_inc_limit)
			batch_size_ = batch_size_ * 3 ;

		covered_feature_row_offset_ += covered_row_size ;
	}

	return log_likelihood_[(corresponding_frame - feature_offset_) * feature_output_dim_ + ((const HMM*)hmm_)->transition2pdf((uint32)index)] ;
}

bool DNNDecodableInterface::is_last_frame(int32 frame)
{
	
	if(!finished_ || frame < 0)  return false ;

	uint32 corresponding_frame = (uint32)(skip_frame ? (frame >> 1) : frame) ;

	return (corresponding_frame >= (feature_offset_ + num_row_ - 1)) ;
}
