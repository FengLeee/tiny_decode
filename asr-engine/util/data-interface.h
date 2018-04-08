/*
 * data-interface.h
 *
 *  Created on : 07/28/2014
 *      Author : zhiming.wang
 */

# ifndef _DATA_INTERFACE_H_
# define _DATA_INTERFACE_H_

# include "mult-thread.h"
# include "queue.h"

class DateInterface
{
public :
	DateInterface() ;
	~DateInterface() ;

	/* initialize */
	void initialize() ;

	/* release */
	void release() ;

	int32 input_data(int16* input , int32 acquire_length) ;

	int32 read(BaseFloat* output , int32 request_length) ;

  	int32 data_finished(void) ;

private :
  	Queue* queue_ ;

    ConditionMutex cond_ ;

    bool is_data_finished_ ;

    bool is_wait_producer_signal_ ;
    bool is_wait_consumer_signal_ ;

    int32 request_length_ ;
    int32 acquire_length_ ;
} ;

# endif /* _DATA_INTERFACE_H_ */
