/*
 * hmm.cc
 *
 *  Created on : 6/21/2014
 *      Author : zhiming.wang
 */

# include "hmm.h"

HMM::HMM() : hmm_quad_(NULL) , state2id_(NULL) , id2state_(NULL)
{
	initialize() ;
}

HMM::~HMM()
{
	release() ;
}

/* initialize */
void HMM::initialize()
{
	hmm_quad_ = NULL ;
	num_hmm_quad_ = 0 ;

	state2id_ = NULL ;
	num_state2id_ = 0 ;

	id2state_ = NULL ;
	num_id2state_ = 0 ;
}

/* release */
void HMM::release()
{
	if(NULL != hmm_quad_)
	{
		free(hmm_quad_) ;
		hmm_quad_ = NULL ;
	}
	num_hmm_quad_ = 0 ;

	if(NULL != state2id_)
	{
		free(state2id_) ;
		state2id_ = NULL ;
	}
	num_state2id_ = 0 ;

	if(NULL != id2state_)
	{
		free(id2state_ ) ;
		id2state_ = NULL ;
	}
	num_id2state_ = 0 ;
}

/* read hmm file */
void HMM::read(const char* hmm_file)
{
	FILE* f = fopen(hmm_file , "rb") ;
	if(NULL == f)
	{
		char err[100] ;
		sprintf(err , "error : failed to open hmm file : %s" , hmm_file) ;
		error(err , __FILE__ , __LINE__) ;
	}

	char str[128] ;
	uint32 phone , hmm_state , pdf , num_trans ;

	fscanf(f , "%s" , str) ;
	assert(0 == strcmp(str , "<HMMTransitionModel>")) ;

	fscanf(f , "%u" , &num_hmm_quad_) ;

	hmm_quad_ = (HMMQuad*)malloc(num_hmm_quad_ * sizeof(HMMQuad)) ;
	if(NULL == hmm_quad_)  error("no memory to be allocated" , __FILE__ , __LINE__) ;

	for(uint32 i = 0 ; i < num_hmm_quad_ ; ++i)
	{
		fscanf(f , "%u" , &phone) ;
		fscanf(f , "%u" , &hmm_state) ;
		fscanf(f , "%u" , &pdf) ;
		fscanf(f , "%u" , &num_trans) ;

		hmm_quad_[i].phone = phone ;
		hmm_quad_[i].hmm_state = hmm_state ;
		hmm_quad_[i].pdf = pdf ;
		hmm_quad_[i].num_trans = num_trans ;
	}

	fscanf(f , "%s" , str) ;
	assert(0 == strcmp(str , "</HMMTransitionModel>")) ;

	fclose(f) ;

	num_state2id_ = num_hmm_quad_ + 2 ;
	state2id_ = (uint32*)malloc(num_state2id_ * sizeof(uint32)) ;  /* indexed by transition-state , which is one based */
	if(NULL == state2id_)  error("no memory to be allocated" , __FILE__ , __LINE__) ;
	memset((void*)state2id_ , 0 , num_state2id_ * sizeof(uint32)) ;

	uint32 cur_transition_id = 1 ;
	num_pdfs_ = 0 ;
	for(uint32 trans_state = 1 ; trans_state <= num_hmm_quad_ + 1 ; trans_state++)
	{
		state2id_[trans_state] = cur_transition_id ;
		if(trans_state <= num_hmm_quad_)
		{
			pdf = hmm_quad_[trans_state - 1].pdf ;
			pdf += 1 ;
			num_pdfs_ = MAX(num_pdfs_ , pdf) ;
			num_trans = hmm_quad_[trans_state - 1].num_trans ;
			cur_transition_id += num_trans ;  /* transition out of this state */
		}
	}

	num_id2state_ = cur_transition_id ;
	id2state_ = (uint32*)malloc(num_id2state_ * sizeof(uint32)) ;
	if(NULL == id2state_)  error("no memory to be allocated" , __FILE__ , __LINE__) ;
	memset((void*)id2state_ , 0 , num_id2state_ * sizeof(uint32)) ;

	for(uint32 trans_state = 1 ; trans_state <= num_hmm_quad_ ; ++trans_state)
	{
		for(uint32 id = state2id_[trans_state] ; id < state2id_[trans_state + 1] ; ++id)
			id2state_[id] = trans_state ;
	}
}

uint32 HMM::num_pdfs() const
{
	return num_pdfs_ ;
}

uint32 HMM::num_transitions() const
{
	return num_id2state_ - 1 ;
}

/* transition id -> state */
uint32 HMM::transition2state(uint32 id) const
{
	assert(id != 0 && id < num_id2state_) ;
	return id2state_[id] ;
}

/* transition id -> pdf */
uint32 HMM::transition2pdf(uint32 id) const
{
	uint32 trans_state = transition2state(id) ;
	return hmm_quad_[trans_state - 1].pdf ;
}

/* transition id -> phone */
uint32 HMM::transition2phone(uint32 id) const
{
	uint32 trans_state = transition2state(id) ;
	return hmm_quad_[trans_state - 1].phone ;
}
