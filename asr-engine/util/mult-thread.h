/*
 * mult-thread.h
 *
 *  Created on : 07/28/2014
 *      Author : zhiming.wang
 */

# ifndef _MULT_THREAD_H_
# define _MULT_THREAD_H_

# include "../base/common.h"

# include <pthread.h>

/* add "-lpthread" as link option */

class Mutex {
public :
	Mutex()  {}
	~Mutex()  {}

	/* initialize */
	void initialize() ;

	/* release */
	void release() ;

	void Lock() ;
	void Unlock() ;

	/*
	 * try to lock the mutex without waiting for it
	 * return : true when locking successfully , false when mutex was already locked
	 */
	bool TryLock() ;

protected :
	pthread_mutex_t mutex_ ;

} ;

/*******************************************************************************************************************/

class ConditionMutex : public Mutex
{
public :
	ConditionMutex()  {}
	~ConditionMutex()  {}

	/* initialize */
	void init() ;

	/* destroy */
	void destroy() ;

	void WaitCond(unsigned int* timeout) ;
	void SignalCond() ;

private :
	pthread_cond_t cond_ ;
	
} ;

# endif /* _MULT_THREAD_H_ */
