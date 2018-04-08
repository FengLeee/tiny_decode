/*
 * faster-decoder.h
 *
 *  Created on : 08/05/2014
 *      Author : zhiming.wang
 */

# ifndef _FASTER_DECODER_H_
# define _FASTER_DECODER_H_

# include "../util/hash-list.h"
# include "dnn-decodable-interface.h"

/*
 * Tropical SemiRing
 * K : [positive infinity , negative infinity]
 * plus : minimize
 * multiply : plus
 * zero : infinity
 * one : zero
 */

struct FasterDecoderOptions
{
	BaseFloat beam_ ;
	BaseFloat beam_delta_ ;
	int32 max_active_ ;
	int32 min_active_ ;
	int32 hash_ratio_ ;
	uint32 batch_size_ ;
	uint32 max_utt_length_ ;
	uint32 inter_sil_length_ ;

	inline FasterDecoderOptions() : beam_(16.0) , beam_delta_(0.5) , max_active_(INT_MAX) , min_active_(20) , hash_ratio_(2) , batch_size_(625) , max_utt_length_(2000) , inter_sil_length_(50)
	{}
	
	inline FasterDecoderOptions& operator=(const FasterDecoderOptions& src)
	{
		beam_ = src.beam_ ;  beam_delta_ = src.beam_delta_ ;
		max_active_ = src.max_active_ ; min_active_ = src.min_active_ ;
		hash_ratio_ = src.hash_ratio_ ;
		batch_size_ = src.batch_size_ ;
		max_utt_length_ = src.max_utt_length_ ;
		inter_sil_length_ = src.inter_sil_length_ ;
		return *this ;
	}
	
	inline void initialize()
	{
		beam_ = 16.0 ;
		beam_delta_ = 0.5 ;
		max_active_ = INT_MAX ;
		min_active_ = 20 ;
		hash_ratio_ = 2 ;
		batch_size_ = 625 ;
		max_utt_length_ = 2000 ;
		inter_sil_length_ = 50 ;
	}

} ;

class FasterDecoder
{
public :
	inline FasterDecoder() : config_(NULL) , fst_(NULL) , tokens_(NULL) , dummy_arc_(NULL)  {}

	FasterDecoder(const FasterDecoderOptions* config , const WFST* fst) ;

	virtual ~FasterDecoder() ;

	typedef HashList::Elem Elem ;

	/* initialize */
	void initialize(const FasterDecoderOptions* config , const WFST* fst) ;

	/* release */
	virtual void release() ;

	bool at_final() ;

	bool get_best_path(VectorArc* vector_arc) ;

	void decode(DNNDecodableInterface* dnn_decodable_interface) ;

	/* return the pruning cutoff */
	BaseFloat process_nonepsilon_transition(DNNDecodableInterface* dnn_decodable_interface , int32 frame) ;

	void process_epsilon_transition(BaseFloat prune_cutoff) ;

	inline void set_config(const FasterDecoderOptions* config)
	{
		config_ = config ;
	}

	inline const FasterDecoderOptions* get_config() const
	{
		return config_ ;
	}

	void clear(Elem* elme_list) ;

protected :
	const FasterDecoderOptions* config_ ;
	const WFST* fst_ ;
	HashList* tokens_ ;
	Arc_* dummy_arc_ ;
	VectorI queue_ ;
	VectorF weight_array_ ;

	inline void resize_hash(size_t number_tokens)
	{
		size_t new_size = (size_t)(number_tokens * config_->hash_ratio_) ;
		if(new_size > tokens_->size())  tokens_->resize(new_size) ;
	}

	/* get the weight cutoff , also counts the active tokens  */
	BaseFloat prune(Elem* list_head , size_t* token_count , BaseFloat* adaptive_beam , Elem** best_elem) ;
	
} ;

# endif /* _FASTER_DECODER_H_ */
