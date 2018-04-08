/*
 * prior.h
 *
 *  Created on : 07/16/2014
 *      Author : zhiming.wang
 */

# ifndef _PRIOR_H_
# define _PRIOR_H_

# include "../base/common.h"

class Prior
{
public :
	Prior() ;
	Prior(const char* prior_pro) ;
	~Prior() ;

	/* initialize */
	void initialize(const char* prior_pro) ;

	/* release */
	void release() ;

	int32 get_dim() const ;

	/* subtract priors from log posterior probabilities to get pseudo log likelihoods */
	void subtract_log_post(BaseFloat* log_likelihood , int32 num_row , int32 num_col , BaseFloat scale) const ;

private :
	BaseFloat* log_prior_ ;
	int32 dim_ ;
	
} ;

# endif /* _PRIOR_H_ */
