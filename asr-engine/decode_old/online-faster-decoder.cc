/*
 * online-faster-decoder.cc
 *
 *  Created on : 08/28/2014
 *      Author : zhiming.wang
 */

# include "online-faster-decoder.h"

int32 binary_search(const VectorI* array , int32 low , int32 high , const int32& target)
{
	if(low > high)  return -1 ;

	int32 middle = low + (high - low) / 2 ;

	if((*array)[(size_t)middle] < target)
		return binary_search(array , middle + 1 , high , target) ;
	else if((*array)[(size_t)middle] == target)
		return middle ;
	else
		return binary_search(array , low , middle - 1 , target) ;

	return -1 ;
}

OnlineFasterDecoder::OnlineFasterDecoder(const FasterDecoderOptions* config , const WFST* fst , const VectorI* sil_phones , const HMM* hmm) :
FasterDecoder(config , fst) , sil_phones_(sil_phones) , hmm_(hmm) , utt_frame_(0) , frame_(0) , state_(EndFeats) , immortal_token_(NULL) , prev_immortal_token_(NULL) , batch_size_(0) , batch_size_inc_(0)
{}

OnlineFasterDecoder::~OnlineFasterDecoder()
{
	sil_phones_ = NULL ;
	hmm_ = NULL ;
	utt_frame_ = 0 ;
	frame_ = 0 ;
	state_ = EndFeats ;
	immortal_token_ = NULL ;
	prev_immortal_token_ = NULL ;
}

/* initialize */
void OnlineFasterDecoder::initialize(const FasterDecoderOptions* config , const WFST* fst , const VectorI* sil_phones , const HMM* hmm)
{
	FasterDecoder::initialize(config , fst) ;
	sil_phones_ = sil_phones ;
	hmm_ = hmm ;
	utt_frame_ = 0 ;
	frame_ = 0 ;
	state_ = EndFeats ;
	immortal_token_ = NULL ;
	prev_immortal_token_ = NULL ;

	batch_size_ = config->batch_size_ ;
 	batch_size_inc_ = 0 ;
}

/* release */
void OnlineFasterDecoder::release()
{
	FasterDecoder::release() ;

	sil_phones_ = NULL ;
	hmm_ = NULL ;
	utt_frame_ = 0 ;
	frame_ = 0 ;
	state_ = EndFeats ;
	immortal_token_ = NULL ;
	prev_immortal_token_ = NULL ;
}

void OnlineFasterDecoder::reset(bool end_feat)
{
	clear(tokens_->clear()) ;
	int32 start_state = (int32)fst_->get_start_state() ;

	dummy_arc_->input_label = dummy_arc_->output_label = 0 ;
	dummy_arc_->arc_weight = 0.0 ;
	dummy_arc_->next_state = start_state ;

	Token* token = NULL ;
	Token::Malloc(&token , dummy_arc_ , NULL) ;
	immortal_token_ = prev_immortal_token_ = token ;
	tokens_->insert(start_state , token) ;

	utt_frame_ = 0 ;
	if(end_feat)  frame_ = 0 ;
}

bool OnlineFasterDecoder::end_of_utt()
{
	uint32 sil_frame = config_->inter_sil_length_ / (1 + utt_frame_ / config_->max_utt_length_) ;

	Token* best_token = NULL ;
	for(Elem* elem = tokens_->get_list() ; elem != NULL ; elem = elem->tail_)
		if(NULL == best_token || *best_token < *(elem->value_))  best_token = elem->value_ ;

	for(Token* token = best_token ; token != NULL && sil_frame > 0 ; token = token->prev_)
	{
		/* consider only the non-epsilon arcs */
		if(0 != token->arc_->input_label)
		{
			int32 phone = (int32)hmm_->transition2phone((uint32)token->arc_->input_label) ;
			if(-1 == binary_search(sil_phones_ , 0 , (int32)(sil_phones_->size()) - 1, phone))  return false ;

			sil_frame-- ;
		}
	}

	return true ;
}

DecodeState OnlineFasterDecoder::decode(DNNDecodableInterface* dnn_decodable_interface)
{
	if(EndUtt == state_ || EndFeats == state_)  reset(EndFeats == state_) ;
	else if(batch_reset())  reset(false) ;
	
	process_epsilon_transition(FLT_MAX) ;
	
	uint32 batch_frame = 0 ;
	
	for( ; !dnn_decodable_interface->is_last_frame(frame_ - 1) && batch_frame < batch_size_ ; frame_++ , batch_frame++ , utt_frame_++)
	{
		BaseFloat prune_cutoff = process_nonepsilon_transition(dnn_decodable_interface , frame_) ;

		process_epsilon_transition(prune_cutoff) ;
	}
	
	if(batch_frame == batch_size_ && !dnn_decodable_interface->is_last_frame(frame_ - 1))
	{
		if(end_of_utt())  state_ = EndUtt ;
		else  state_ = EndBatch ;
	} else {
		state_ = EndFeats ;
	}
	
//	if((++batch_size_inc_) <= batch_size_inc_limit)   batch_size_ *= 2 ;
	
	return state_ ;
}

bool OnlineFasterDecoder::finish_trace_back(VectorI& phone , VectorI& word)
{
	if(!phone.empty())  phone.release() ;
	if(!word.empty())  word.release() ;

	Token* best_token = NULL ;

	bool is_at_final = at_final() ;

	if(!is_at_final)
	{
		for(Elem* elem = tokens_->get_list() ; elem != NULL ; elem = elem->tail_)
			if(NULL == best_token || *best_token < *(elem->value_))  best_token = elem->value_ ;
	} else {
		BaseFloat best_weight = INFINITY ;
		for(Elem* elem = tokens_->get_list() ; elem != NULL ; elem = elem->tail_)
		{
			BaseFloat weight = Token::Times(elem->value_->weight_ , fst_->state((int64)elem->key_).weight) ;
			if(ISFINITE(weight) && weight < best_weight)
			{
				best_weight = weight ;
				best_token = elem->value_ ;
			}
		}
	}

	if(NULL == best_token)  return false ;

	/*
	 * trace back
	 * arcs in reverse order
	 */
	VectorArc arcs_reverse ;
	for(Token* token = best_token ; token != immortal_token_ ; token = token->prev_)
		arcs_reverse.push_back(token->arc_) ;

	if(arcs_reverse.back()->next_state == (int32)fst_->get_start_state())  arcs_reverse.pop_back() ;

	for(Arc_** arc = arcs_reverse.end() ;; arc--)
	{	
		if(0 != (*arc)->input_label)  phone.push_back((int32)(hmm_->transition2phone((uint32)(((*arc)->input_label))))) ;

		/* remove local epsilon */
		if(0 != (*arc)->output_label)  word.push_back((*arc)->output_label) ;
		
		if(arc == arcs_reverse.begin())  break ;
	}

	return true ;
}

bool OnlineFasterDecoder::local_trace_back(VectorI& phone , VectorI& word)
{
	UnorderedSet emit(tokens_->elem_length() * 2) ;
	for(Elem* elem = tokens_->get_list() ; elem != NULL ; elem = elem->tail_)
	{
		Token* token = elem->value_ ;
		while(token && 0 == token->arc_->input_label)  token = token->prev_ ;
		if(token)  emit.insert(token) ;
	}

	Token* target = NULL ;
	size_t elem_length = 0 ;
	UnorderedSet::Elem* last_tokens = NULL ;

	while(true)
	{
		if(1 == emit.elem_length())
		{
			target = (emit.head())->token_ ;
			break ;
		}

		if(0 == emit.elem_length())  break ;

		elem_length = emit.elem_length() ;

		last_tokens = emit.clear() ;

		emit.resize(elem_length * 2) ;

		for(UnorderedSet::Elem* elem = last_tokens ; elem != NULL ; elem = elem->tail_)
		{
			Token* token = elem->token_ ;

			Token* prev_token = token->prev_ ;

			while(prev_token && 0 == prev_token->arc_->input_label)  prev_token = prev_token->prev_ ;

			if(NULL == prev_token)  continue ;

			emit.insert(prev_token) ;
		} /* end for loop */

	} /* end while loop */

	if(NULL != target)
	{
		prev_immortal_token_ = immortal_token_ ;
		immortal_token_ = target ;
	}

	if(immortal_token_ == prev_immortal_token_)  return false ;
	
	if(!phone.empty())  phone.release() ;
	if(!word.empty())  word.release() ;

	/*
	 * trace back
	 * arcs in reverse order
	 */
	VectorArc arcs_reverse ;
	for(Token* token = immortal_token_ ; token != prev_immortal_token_ ; token = token->prev_)
		arcs_reverse.push_back(token->arc_) ;

	if(arcs_reverse.back()->next_state == (int32)fst_->get_start_state())  arcs_reverse.pop_back() ;

	for(Arc_** arc = arcs_reverse.end() ;; arc--)
	{
		if(0 != (*arc)->input_label)  phone.push_back((int32)(hmm_->transition2phone((uint32)(((*arc)->input_label))))) ;
		
		/* remove local epsilon */
		if(0 != (*arc)->output_label)  word.push_back((*arc)->output_label) ;

		if(arc == arcs_reverse.begin())  break ;
	}

	return true ;
}
