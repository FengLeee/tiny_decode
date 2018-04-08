/*
 * dnn-decodable-interface.h
 *
 *  Created on : 07/30/2014
 *      Author : zhiming.wang
 */

# ifndef _DNN_DECODABLE_INTERFACE_H_
# define _DNN_DECODABLE_INTERFACE_H_

# include "feature-input.h"
# include "../hmm/hmm.h"
# include "../net/net-interface.h"

class DNNDecodableInterface
{
public :
	DNNDecodableInterface() ;
	DNNDecodableInterface(FilterBankInput* filter_bank_input , const NetInterface* net_interface , const HMM* hmm , BaseFloat acoustic_scale ,  uint32 batch_size) ;
	~DNNDecodableInterface() ;

	/* initialize */
	void initialize(FilterBankInput* filter_bank_input , const NetInterface* net_interface , const HMM* hmm , BaseFloat acoustic_scale , uint32 batch_size) ;

	/* release */
	void release() ;

	/* index are one-based , for compatibility with OpenFst */
	uint32 num_transitions()  const ;

	/* index means transition id */
	BaseFloat log_likelihood(int32 frame , int32 index) ;

	bool is_last_frame(int32 frame) ;

private :
	BaseFloat* log_likelihood_ ;
	FilterBankInput* filter_bank_input_ ;
	const NetInterface* net_interface_ ;               /* net interface */
	const HMM* hmm_ ;                                  /* hmm model */
	BaseFloat acoustic_scale_ ;
	uint32 batch_size_ ;					  /* how many features to request or process in one going round */
	uint32 batch_size_inc_ ;
	uint32 num_row_ ;
	uint32 feature_input_dim_ ;		                /* dimensionality of the input features */
	uint32 feature_output_dim_ ;			  /* dimensionality of the output features */
	uint32 feature_offset_ ;                           /* the offset of the first frame in the current batch */
	bool finished_ ;                                   /* is the input feature already exhausted ? */
	bool skip_frame ;                                  /* is to skip every two frame ? */
	
	static const uint32 batch_size_inc_limit = 4 ;
	
	BaseFloat* net_feature_buffer_ ;
	int32 feature_row_offset_ ;
	int32 covered_feature_row_offset_ ;
	int32 buffer_size_ ;
	int32 buffer_size_inc_ ;
 
} ;

# endif /* _DNN_DECODABLE_INTERFACE_H_ */
