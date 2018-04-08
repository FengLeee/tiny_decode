/*
 * component.cc
 *
 *  Created on : 07/07/2014
 *      Author : zhiming.wang
 */

# include "component.h"

MLPComponent::MLPComponent() : input_dim_(0) , output_dim_(0) , weight_(NULL) , bias_(NULL)
{
	initialize() ;
}

MLPComponent::~MLPComponent()
{
	release() ;
}

/* initialize */
void MLPComponent::initialize()
{
	input_dim_ = 0 ;
	output_dim_ = 0 ;
	weight_ = NULL ;
	bias_ = NULL ;
}

/* release */
void MLPComponent::release()
{
	AlignedFree(&weight_) ;
	AlignedFree(&bias_) ;

	input_dim_ = 0 ;
	output_dim_ = 0 ;
}

/* read in neural net model */
void MLPComponent::read(FILE* f)
{
	assert(NULL != f) ;

	char str[512] ;

	fscanf(f , "%d" , &output_dim_) ;
	fscanf(f , "%d" , &input_dim_) ;

	AlignedAlloc(&weight_ , sizeof(BaseFloat) * output_dim_ * input_dim_) ;
	AlignedAlloc(&bias_ , sizeof(BaseFloat) * output_dim_) ;

	/* read in weight matrix */
	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("[" , str)) ;

	int32 i = 0 ;
	while(i < output_dim_ * input_dim_)  fscanf(f, "%f" , &weight_[i++]) ;

	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("]" , str)) ;

	/* read in bias vector */
	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("[" , str)) ;

	i = 0 ;
	while(i < output_dim_)  fscanf(f, "%f" , &bias_[i++]) ;

	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("]" , str)) ;
}

/* read in binary neural net model */
void MLPComponent::binary_read(FILE* f)
{
	assert(NULL != f) ;

	char str[512] ;
	int32 num_row , num_col , dim_size ;

	fread(str , 1 , 1 , f) ;

	/* output dim */
	fread(str , 1 , 1 , f) ;
	fread((char*)&output_dim_ , 1 , sizeof(int32) , f) ;

	/* input dim */
	fread(str , 1 , 1 , f) ;
	fread((char*)&input_dim_ , 1 , sizeof(int32) , f) ;

	AlignedAlloc(&weight_ , sizeof(BaseFloat) * output_dim_ * input_dim_) ;
	AlignedAlloc(&bias_ , sizeof(BaseFloat) * output_dim_) ;

	/* read in weight matrix */
	/* FM or DM */
	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("FM" , str) || 0 == strcmp("DM" , str)) ;

	fread(str , 1 , 1 , f) ;

	/* row dim */
	fread(str , 1 , 1 , f) ;
	fread((char*)&num_row , 1 , sizeof(int32) , f) ;
	assert(output_dim_ == num_row) ;

	/* column dim */
	fread(str , 1 , 1 , f) ;
	fread((char*)&num_col , 1 , sizeof(int32) , f) ;
	assert(input_dim_ == num_col) ;

	fread((char*)weight_ , 1 , sizeof(BaseFloat) * output_dim_ * input_dim_ , f) ;

	/* read in bias vector */
	/* FV or DV */
	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("FV" , str) || 0 == strcmp("DV" , str)) ;

	fread(str , 1 , 1 , f) ;

	/* dim size */
	fread(str , 1 , 1 , f) ;
	fread((char*)&dim_size , 1 , sizeof(int32) , f) ;
	assert(output_dim_ == dim_size) ;

	fread((char*)bias_ , 1 , sizeof(BaseFloat) * output_dim_ , f) ;
}

int32 MLPComponent::input_dim() const
{
	return input_dim_ ;
}

int32 MLPComponent::output_dim() const
{
	return output_dim_ ;
}

/* forward propagation , with ReLU as active function */
void MLPComponent::forward(const BaseFloat* input , int32 num_row , int32 num_col , BaseFloat* output) const
{
	assert(num_col == input_dim_) ;
	assert(NULL != output) ;

	MatrixMul(num_row , output_dim_ , num_col , input , weight_ , output) ;

	for(int32 i = 0 ; i < num_row ; ++i)
		for(int32 j = 0 ; j < output_dim_ ; ++j)
		{
			output[i * output_dim_ + j] += bias_[j] ;

			/* with ReLU as activation function , apply it */
			output[i * output_dim_ + j] = MAX(output[i * output_dim_ + j] , 0.0f) ;
		}
}

/* apply soft max regression */
void MLPComponent::softmax(const BaseFloat* input , int32 num_row , int32 num_col , BaseFloat* output) const
{
	assert(num_col == input_dim_) ;
	assert(NULL != output) ;

	MatrixMul(num_row , output_dim_ , num_col , input , weight_ , output) ;

	for(int32 i = 0 ; i < num_row ; ++i)
	{
		for(int32 j = 0 ; j < output_dim_ ; ++j)
		{
			output[i * output_dim_ + j] += bias_[j] ;
		}
	}
}
