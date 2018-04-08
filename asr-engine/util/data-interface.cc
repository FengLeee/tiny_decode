/*
 * data-interface.cc
 *
 *  Created on : 07/28/2014
 *      Author : zhiming.wang
 */

# include "data-interface.h"

DateInterface::DateInterface() : queue_(NULL) , is_data_finished_(false) , is_wait_producer_signal_(false) , is_wait_consumer_signal_(false) , request_length_(0) , acquire_length_(0)
{
	initialize() ;
}

DateInterface::~DateInterface()
{
	release() ;
}

/* initialize */
void DateInterface::initialize()
{
	is_data_finished_ = is_wait_producer_signal_ = is_wait_consumer_signal_ = false ;
	request_length_ = acquire_length_ = 0 ;
	
	queue_ = (Queue*)malloc(sizeof(Queue)) ;
	if(NULL == queue_)  error("error : could not allocate memory for queue" , __FILE__ , __LINE__) ;
	queue_->initialize() ;

	cond_.init() ;
}

/* release */
void DateInterface::release()
{
	if(NULL != queue_)
	{
		queue_->release() ;
		free(queue_) ;
		queue_ = NULL ;
	}

	cond_.destroy() ;
}

int32 DateInterface::input_data(int16* input , int32 acquire_length)
{
	
	assert(NULL != input) ;

	cond_.Lock() ;

	if(false == queue_->push_sequence_allowed(acquire_length))
	{
		is_wait_consumer_signal_ = true ;

		acquire_length_ = acquire_length ;

		cond_.WaitCond(NULL) ;

		is_wait_consumer_signal_ = false ;
	}

	queue_->push_sequence(input , acquire_length) ;
	
	

	if(is_wait_producer_signal_ && true == queue_->pop_sequence_allowed(request_length_))
		cond_.SignalCond() ;

	cond_.Unlock() ;

	return 1 ;
}

int32 DateInterface::read(BaseFloat* output , int32 request_length)
{
	static int j = 0;
	assert(NULL != output) ;

	cond_.Lock() ;

	if(false == queue_->pop_sequence_allowed(request_length) && !is_data_finished_)
	{
		is_wait_producer_signal_ = true ;

		request_length_ = request_length ;

		cond_.WaitCond(NULL) ;

		is_wait_producer_signal_ = false ;
	}

	size_t num_element = MIN(queue_->size() , (size_t)request_length) ;

	queue_->pop_sequence(output , num_element) ;

	if(is_wait_consumer_signal_ && true == queue_->push_sequence_allowed(acquire_length_))
		cond_.SignalCond() ;

	cond_.Unlock() ;
	
	return (int32)num_element ;
}

int32 DateInterface::data_finished(void)
{
	cond_.Lock() ;

	is_data_finished_ = true ;

	if(is_wait_producer_signal_)  cond_.SignalCond() ;

	cond_.Unlock() ;

	return 1 ;
}
