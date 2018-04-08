/*
 * prior.cc
 *
 *  Created on : 07/16/2014
 *      Author : zhiming.wang
 */

# include "prior.h"

Prior::Prior() : log_prior_(NULL) , dim_(0)  {}

Prior::Prior(const char* prior_pro) : log_prior_(NULL), dim_(0)
{
	initialize(prior_pro) ;
}

Prior::~Prior()
{
	release() ;
}

/* release */
void Prior::release()
{
	if(NULL != log_prior_)
	{
		free(log_prior_) ;  log_prior_ = NULL ;
	}
}

/* initialize */
void Prior::initialize(const char* prior_pro)
{
	FILE* f = fopen(prior_pro , "rb") ;
	if(NULL == f)
	{
		char err[100] ;
		sprintf(err , "error : failed to open neural net file : %s" , prior_pro) ;
		error(err , __FILE__ , __LINE__) ;
	}

	char str[512] ;

	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("[" , str)) ;

	dim_ = 0 ;
	const int32 buffer_size_inc = 5000 ;
	int32 buffer_size = buffer_size_inc ;
	double64* buffer = (double64*)malloc(sizeof(double64) * buffer_size) ;
	assert(NULL != buffer) ;
	double64 sum = 0.0 ;

	while(!feof(f))
	{
		fscanf(f , "%s" , str) ;

		if(0 == strcmp("]" , str))  break ;

		buffer[dim_] = atof(str) ;

		sum += buffer[dim_] ;

		if((++dim_) == buffer_size)
		{
			buffer_size += buffer_size_inc ;
			buffer = (double64*)realloc(buffer , sizeof(double64) * buffer_size) ;
			assert(NULL != buffer) ;
		}
	}

	assert(0.0 != sum) ;

	log_prior_ = (BaseFloat*)malloc(dim_ * sizeof(BaseFloat)) ;
	assert(NULL != log_prior_) ;

	double64 prior_cutoff = 1.0 ;
	int32 num_cutoff = 0 ;

	for(int32 i = 0 ; i < dim_ ; i++)
	{
		if(buffer[i] < prior_cutoff)
		{
			log_prior_[i] = -0.5 * FLT_MAX ;
			num_cutoff++ ;
		}else {
			log_prior_[i] = (BaseFloat)(log(buffer[i] / sum)) ;
		}
	}

	if(num_cutoff > 0)
		printf("warn : %d out of %d classes have counts lower than 1 , %s , line : %d \n" , num_cutoff , dim_ , __FILE__ , __LINE__) ;

	fclose(f) ;

	free(buffer) ; buffer = NULL ;
}

int32 Prior::get_dim() const
{
	return dim_ ;
}

/* subtract priors from log posterior probabilities to get pseudo log likelihoods */
void Prior::subtract_log_post(BaseFloat* log_likelihood , int32 num_row , int32 num_col , BaseFloat scale) const
{
	if(0 == dim_)
		error("class frame counts is empty : cannot initialize priors without the counts \n" , __FILE__ , __LINE__) ;

	if(dim_ != num_col)
	{
		char err[100] ;
		sprintf(err , "error : dimensionality mismatch , priors : %d , log likelihoods : %d \n" , dim_ , num_col) ;
		error(err , __FILE__ , __LINE__) ;
	}

	for(int32 i = 0 ; i < num_row ; ++i)
		for(int32 j = 0 ; j < num_col ; ++j)
			log_likelihood[i * num_col + j] = (log_likelihood[i * num_col + j] - log_prior_[j]) * scale ;
}
