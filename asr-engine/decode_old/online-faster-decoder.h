/*
 * online-faster-decoder.h
 *
 *  Created on : 08/28/2014
 *      Author : zhiming.wang
 */

# ifndef _ONLINE_FASTER_DECODER_H_
# define _ONLINE_FASTER_DECODER_H_

# include "faster-decoder.h"
# include "../util/unordered-set.h"

 enum DecodeState
 {
	 EndFeats = 1 ,
	 EndUtt = 2 ,
	 EndBatch = 4
 } ;

int32 binary_search(const VectorI* array , int32 low , int32 high , const int32& target) ;

class OnlineFasterDecoder : public FasterDecoder
{
public :
	inline OnlineFasterDecoder() : sil_phones_(NULL) , hmm_(NULL) , utt_frame_(0) , frame_(0) , state_(EndFeats) , immortal_token_(NULL) , prev_immortal_token_(NULL) , batch_size_(0) , batch_size_inc_(0)  {}

	OnlineFasterDecoder(const FasterDecoderOptions* config , const WFST* fst , const VectorI* sil_phones , const HMM* hmm) ;

	~OnlineFasterDecoder() ;

	/* initialize */
	void initialize(const FasterDecoderOptions* config , const WFST* fst , const VectorI* sil_phones , const HMM* hmm) ;

	/* release */
	virtual void release() ;

	void reset(bool end_feat) ;

	bool end_of_utt() ;

	DecodeState decode(DNNDecodableInterface* dnn_decodable_interface) ;

	bool finish_trace_back(VectorI& phone , VectorI& word) ;

	bool local_trace_back(VectorI& phone , VectorI& word) ;

	inline bool batch_reset() const
	{
		return EndBatch == state_ && utt_frame_ > 10 * config_->max_utt_length_ ;
	}
	
	inline uint32 get_frame()
	{
		return frame_ ;
	}

private :
	const VectorI* sil_phones_ ;
	const HMM* hmm_ ;

	uint32 utt_frame_ ;
	uint32 frame_ ;

	DecodeState state_ ;

	Token* immortal_token_ ;
	Token* prev_immortal_token_ ;
	
	uint32 batch_size_ ;					     
	uint32 batch_size_inc_ ;
	static const uint32 batch_size_inc_limit = 4 ;

} ;

# endif /* _ONLINE_FASTER_DECODER_H_ */
