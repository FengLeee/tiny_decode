/*
 * faster-decoder.cc
 *
 *  Created on : 08/05/2014
 *      Author : zhiming.wang
 */

# include "faster-decoder.h"

FasterDecoder::FasterDecoder(const FasterDecoderOptions* config , const WFST* fst) : config_(NULL) , fst_(NULL) , tokens_(NULL) , dummy_arc_(NULL)
{
	initialize(config , fst) ;
}

FasterDecoder::~FasterDecoder()
{
	release() ;
}

/* initialize */
void FasterDecoder::initialize(const FasterDecoderOptions* config , const WFST* fst)
{
	config_ = config ;
	fst_ = fst ;

	tokens_ = (HashList*)malloc(sizeof(HashList)) ;
	if(NULL == tokens_)  error("error to allocate memory for token hash list" , __FILE__ , __LINE__) ;
	tokens_->initialize() ;
	tokens_->resize(1000) ;

	assert(config_->hash_ratio_ >= 1.0) ;
	assert(config_->max_active_ > 1) ;
	assert(config_->min_active_ >= 0 && config_->min_active_ < config_->max_active_) ;

	dummy_arc_ = (Arc_*)malloc(sizeof(Arc_)) ;
	if(NULL == dummy_arc_)  error("error to allocate memory for dummy arc" , __FILE__ , __LINE__) ;
}

/* release */
void FasterDecoder::release()
{
	if(NULL != tokens_)
	{
		/*
		for(Elem* elem = tokens_->get_list() ; elem != NULL ; elem = elem->tail_)
		{
			for(Token* token = elem->value_ ; token != NULL ; token = token->prev_)
			Token::Delete(token) ;
		}
		*/
		clear(tokens_->clear()) ;
		tokens_->release() ;
		free(tokens_) ;
		tokens_ = NULL ;
	}

	if(NULL != dummy_arc_)
	{
		free(dummy_arc_) ;
		dummy_arc_ = NULL ;
	}
}

bool FasterDecoder::at_final()
{
	for(Elem* elem = tokens_->get_list() ; elem != NULL ; elem = elem->tail_)
	{
		BaseFloat weight = Token::Times(elem->value_->weight_ , fst_->state((int64)elem->key_).weight) ;
		if(ISFINITE(weight))  return true ;
	}
	return false ;
}

bool FasterDecoder::get_best_path(VectorArc* vector_arc)
{
	assert(NULL != vector_arc) ;

	vector_arc->release() ;

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
	for(Token* token = best_token ; token != NULL ; token = token->prev_)
		arcs_reverse.push_back(token->arc_) ;

	assert(arcs_reverse.back()->next_state == (int32)fst_->get_start_state()) ;
	arcs_reverse.pop_back() ;

	for(Arc_** arc = arcs_reverse.end() ;; arc--)
	{
		/* remove local epsilon */
		if(0 != (*arc)->output_label)  vector_arc->push_back(*arc) ;

		if(arc == arcs_reverse.begin())  break ;
	}

	return true ;
}

void FasterDecoder::decode(DNNDecodableInterface* dnn_decodable_interface)
{
	clear(tokens_->clear()) ;
	int32 start_state = (int32)fst_->get_start_state() ;

	dummy_arc_->input_label = dummy_arc_->output_label = 0 ;
	dummy_arc_->arc_weight = 0.0 ;
	dummy_arc_->next_state = start_state ;

	Token* token = NULL ;
	Token::Malloc(&token , dummy_arc_ , NULL) ;
	tokens_->insert(start_state , token) ;

	process_epsilon_transition(FLT_MAX) ;

	for(int32 frame = 0 ; !dnn_decodable_interface->is_last_frame(frame - 1) ; frame++)
	{
		BaseFloat prune_cutoff = process_nonepsilon_transition(dnn_decodable_interface , frame) ;
		process_epsilon_transition(prune_cutoff) ;
	}
}

/* return the pruning cutoff */
BaseFloat FasterDecoder::process_nonepsilon_transition(DNNDecodableInterface* dnn_decodable_interface , int32 frame)
{
	Elem* last_tokens = tokens_->clear() ;  /* the hash list is empty */
	size_t number_tokens = 0 ;
	BaseFloat adaptive_beam ;
	Elem* best_elem = NULL ;
	
	BaseFloat prune_cutoff = prune(last_tokens , &number_tokens , &adaptive_beam , &best_elem) ;
	
	resize_hash(number_tokens) ;

	BaseFloat next_prune_cutoff = INFINITY ;
	
	/* first process the best token to get a hopefully reasonably tight bound on the next cutoff */
	if(best_elem)
	{
		int32 best_state = best_elem->key_ ;
		Token* best_token = best_elem->value_ ;

		const StateNode& best_state_node = fst_->state((int64)best_state) ;

		for(int64 j = 0 ; j < best_state_node.num_arc ; ++j)
		{
			Arc_& arc = best_state_node.arc[j] ;
			/* process non-epsilon transition */
			if(0 != arc.input_label)
			{
				BaseFloat acoustic_cost = -dnn_decodable_interface->log_likelihood(frame , arc.input_label) ;
				BaseFloat weight_cost = best_token->weight_ + arc.arc_weight + acoustic_cost ;

				if(weight_cost + adaptive_beam < next_prune_cutoff)  next_prune_cutoff = weight_cost + adaptive_beam ;
			}  /* end if condition */
		}
	}  /* end if condition */

	for(Elem *elem = last_tokens , *elem_tail = NULL ; elem != NULL ; elem = elem_tail)
	{
		int32 state = elem->key_ ;
		Token* token = elem->value_ ;

		/* not pruned */
		if(token->weight_ < prune_cutoff)
		{
			assert(state == token->arc_->next_state) ;

			const StateNode& state_node = fst_->state((int64)state) ;

			for(int64 j = 0 ; j < state_node.num_arc ; ++j)
			{
				Arc_& arc = state_node.arc[j] ;

				/* process non-epsilon transition */
				if(0 != arc.input_label)
				{
					BaseFloat acoustic_cost = -dnn_decodable_interface->log_likelihood(frame , arc.input_label) ;
					BaseFloat weight_cost = token->weight_  + arc.arc_weight + acoustic_cost ;

					/* not pruned */
					if(weight_cost < next_prune_cutoff)
					{
						if(weight_cost + adaptive_beam < next_prune_cutoff)  next_prune_cutoff = weight_cost + adaptive_beam ;

						Token* new_token = NULL ;
						Token::Malloc(&new_token , &arc , acoustic_cost , token) ;
						Elem* elem_found = tokens_->find(arc.next_state) ;

						if(NULL == elem_found)
						{
							tokens_->insert(arc.next_state , new_token) ;
						} else {
							if(*(elem_found->value_) < *new_token)
							{
								Token::Delete(elem_found->value_) ;
								elem_found->value_ = new_token ;
							} else
								Token::Delete(new_token) ;
						}  /* end if condition */
					}  /* end if condition */
				}  /* end if condition */
			}  /* end for loop */
		}  /* end if condition */
		elem_tail = elem->tail_ ;
		Token::Delete(elem->value_) ;
		tokens_->delete_elem(elem) ;
	}  /* end for loop */

	return next_prune_cutoff ;
}

void FasterDecoder::process_epsilon_transition(BaseFloat prune_cutoff)
{
	assert(queue_.empty()) ;

	Elem* elem = NULL ;
	Token* token = NULL ;

	for(elem = tokens_->get_list() ; elem != NULL ; elem = elem->tail_)
		queue_.push_back(elem->key_) ;

	while(!queue_.empty())
	{
		int32 state = queue_.back() ;
		queue_.pop_back() ;

		elem = tokens_->find(state) ;
		if(NULL != elem) token = elem->value_ ;
		else  error("could not find token (corresponding to given state) in tokens hash list" , __FILE__ , __LINE__) ;  /* but impossible to happen */

		if(token->weight_ > prune_cutoff)  continue ;

		assert(NULL != token && state == token->arc_->next_state) ;

		const StateNode& state_node = fst_->state((int64)state) ;

		for(int64 j = 0 ; j < state_node.num_arc ; ++j)
		{
			Arc_& arc = state_node.arc[j] ;

			/* process epsilon transition */
			if(0 == arc.input_label)
			{
				Token* new_token = NULL ;
				Token::Malloc(&new_token , &arc , token) ;

				if(new_token->weight_ > prune_cutoff)
				{
					Token::Delete(new_token) ;
				} else {
					Elem* elem_found = tokens_->find(arc.next_state) ;

					if(NULL == elem_found)
					{
						tokens_->insert(arc.next_state , new_token) ;
						queue_.push_back(arc.next_state) ;
					} else {
						if(*(elem_found->value_) < *new_token)
						{
							Token::Delete(elem_found->value_) ;
							elem_found->value_ = new_token ;
							queue_.push_back(arc.next_state) ;
						} else
							Token::Delete(new_token) ;
					}  /* end if ... else ... condition */
				}  /* end if ... else ... condition */
			}  /* end outer if condition */
		}  /* end for loop */
	}  /* end while loop */
}

/* get the weight cutoff , also count the active tokens  */
BaseFloat FasterDecoder::prune(Elem* list_head , size_t* token_count , BaseFloat* adaptive_beam , Elem** best_elem)
{
	BaseFloat best_weight = INFINITY ;
	size_t count = 0 ;

	if(INT_MAX == config_->max_active_ && 0 == config_->min_active_)
	{
		for(Elem* elem = list_head ; elem != NULL ; elem = elem->tail_ , count++)
		{
			BaseFloat weight = elem->value_->weight_ ;
			if(weight < best_weight)
			{
				best_weight = weight ;
				if(best_elem)  *best_elem = elem ;
			}
		}
		if(token_count)  *token_count = count ;
		if(adaptive_beam)  *adaptive_beam = config_->beam_ ;
		return best_weight + config_->beam_ ;
	}
	
	if(!weight_array_.empty())  weight_array_.clear() ;
	
	for(Elem* elem = list_head ; elem != NULL ; elem = elem->tail_ , count++)
	{
		BaseFloat weight = elem->value_->weight_ ;
		
		weight_array_.push_back(weight) ;
		
		if(weight < best_weight)
		{
			best_weight = weight ;
			if(best_elem)  *best_elem = elem ;
		}
	}
	
	if(token_count)  *token_count = count ;
	
	BaseFloat beam_cutoff = best_weight + config_->beam_ ;

	if(weight_array_.size() > (size_t)config_->max_active_)
	{
		nthElement(weight_array_.begin() , 0 , (int32)(weight_array_.size() - 1) , config_->max_active_) ;

		BaseFloat max_active_cutoff = weight_array_[config_->max_active_] ;

		/* max_active is tighter than beam */
		if(max_active_cutoff < beam_cutoff)
		{
			if(adaptive_beam)  *adaptive_beam = max_active_cutoff - best_weight + config_->beam_delta_ ;
			return max_active_cutoff ;
		}
	}

	if(weight_array_.size() > (size_t)config_->min_active_)
	{
		BaseFloat min_active_cutoff ;
		if(0 == config_->min_active_)  min_active_cutoff = best_weight ;
		else {
			nthElement(weight_array_.begin() , 0 ,  \
				    (int32)(weight_array_.size() > (size_t)config_->max_active_ ? config_->max_active_ - 1 : weight_array_.size() - 1) ,  \
				     config_->min_active_) ;

			min_active_cutoff = weight_array_[config_->min_active_] ;
		}

		/* min_active is looser than beam */
		if (min_active_cutoff > beam_cutoff)
		{
			if(adaptive_beam)  *adaptive_beam = min_active_cutoff - best_weight + config_->beam_delta_ ;
			return min_active_cutoff ;
		}
	}

	if(adaptive_beam)  *adaptive_beam = config_->beam_ ;

	return beam_cutoff ;
}

void FasterDecoder::clear(Elem* elem_list)
{
	for(Elem *elem = elem_list , *tail = NULL ; elem != NULL ; elem = tail)
	{
		tail = elem->tail_ ;
		Token::Delete(elem->value_) ;
		tokens_->delete_elem(elem) ;
	}
}
