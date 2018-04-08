/*
 * net-input.cc
 *
 *  Created on : 07/17/2014
 *      Author : zhiming.wang
 */

# include "net-input.h"

NetInput::NetInput() : input_dim_(0) , output_dim_(0) , splice_dim_(0) , skip_frame_(false) , splice_(NULL) , shift_(NULL) , scale_(NULL)  
{}

NetInput::NetInput(const char* transform_file , bool skip_frame , BaseFloat scale_factor) : input_dim_(0) , output_dim_(0) , splice_dim_(0) , skip_frame_(false) , splice_(NULL) , shift_(NULL) , scale_(NULL) 
{
	initialize(transform_file , skip_frame , scale_factor) ;
}

NetInput::~NetInput()
{
	release() ;
}

/* release */
void NetInput::release()
{
	if(NULL != splice_)
	{
		free(splice_) ; splice_ = NULL ;
	}

	if(NULL != shift_)
	{
		free(shift_) ; shift_ = NULL ;
	}

	if(NULL != scale_)
	{
		free(scale_) ; scale_ = NULL ;
	}
		
	input_dim_ = output_dim_ = splice_dim_ = 0 ;
}

bool NetInput::skipping_frame() const
{
	return skip_frame_ ;
}

/* initialize */
void NetInput::initialize(const char* transform , bool skip_frame , BaseFloat scale_factor)
{
	skip_frame_ = skip_frame ;

	FILE* f = fopen(transform , "rb") ;
	if(NULL == f)
	{
		char err[100] ;
		sprintf(err , "error : failed to open neural net file : %s" , transform) ;
		error(err , __FILE__ , __LINE__) ;
	}

	char str[512] ;
	int32 dim = 0 ;

	/* <splice> */
	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("<splice>" , str)) ;

	fscanf(f , "%d" , &output_dim_) ;
	fscanf(f , "%d" , &input_dim_) ;

	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("[" , str)) ;

	const int32 buffer_size_inc = 100 ;
	int32 buffer_size = buffer_size_inc ;
	int32* buffer = (int32*)malloc(sizeof(int32) * buffer_size) ;
	assert(NULL != buffer) ;

	left_context_ = 0 ;
	right_context_ = 0 ;
	int32 index = 0 ;

	while(!feof(f))
	{
		fscanf(f , "%s" , str) ;

		if(0 == strcmp("]" , str))  break ;
		
		index = atoi(str) ;
		
		if(index < 0)  left_context_++ ;
		if(index > 0)  right_context_++ ;

		buffer[dim++] = index ;

		if(dim == buffer_size)
		{
			buffer_size += buffer_size_inc ;
			buffer = (int32*)realloc(buffer , sizeof(int32) * buffer_size) ;
			assert(NULL != buffer) ;
		}
	}

	splice_ = (int32*)malloc(sizeof(int32) * dim) ;
	assert(NULL != splice_) ;

	splice_dim_ = dim ;
	memcpy((void*)splice_ , (const void*)buffer , sizeof(int32) * dim) ;

	free(buffer) ; buffer = NULL ;

	/* <addshift> */
	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("<addshift>" , str)) ;

	fscanf(f , "%d" , &dim) ;
	assert(dim == output_dim_) ;

	fscanf(f , "%d" , &dim) ;
	assert(dim == output_dim_) ;

	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("[" , str)) ;

	shift_ = (BaseFloat*)malloc(output_dim_ * sizeof(BaseFloat)) ;
	assert(NULL != shift_) ;

	int32 i = 0 ;
	for(; i < output_dim_ ; ++i)
		fscanf(f , "%f" , &shift_[i]) ;

	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("]" , str)) ;

	/* <rescale> */
	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("<rescale>" , str)) ;

	fscanf(f , "%d" , &dim) ;
	assert(dim == output_dim_) ;

	fscanf(f , "%d" , &dim) ;
	assert(dim == output_dim_) ;

	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("[" , str)) ;

	scale_ = (BaseFloat*)malloc(output_dim_ * sizeof(BaseFloat)) ;
	assert(NULL != scale_) ;

	for(i = 0 ; i < output_dim_ ; ++i)
		fscanf(f , "%f" , &scale_[i]) ;

	fscanf(f , "%s" , str) ;
	assert(0 == strcmp("]" , str)) ;

	if(1.0 != scale_factor)
		for(i = 0 ; i < output_dim_ ; ++i)  scale_[i] *= scale_factor ;

	fclose(f) ;
}

int32 NetInput::input_dim() const
{
	return input_dim_ ;
}

int32 NetInput::output_dim() const
{
	return output_dim_ ;
}

/* net input transform */
void NetInput::forward(const BaseFloat* input , int32 num_row , int32 num_col , BaseFloat* output , int32 out_num_row , int32 out_num_col , int32 feature_row_offset , int32 covered_feature_row_offset , uint32 covered_row_size)  const
{
	assert(num_col == input_dim_) ;
	assert(out_num_col == output_dim_) ;

	int32 row , col ;
	BaseFloat temp ;

	if(false == skip_frame_)
	{
		for(int32 r = covered_feature_row_offset ; r < covered_feature_row_offset + (int32)(covered_row_size) ; ++r)
		{
			for(int32 i = 0 ; i < splice_dim_ ; ++i)
			{
				row = r + splice_[i] ;
				if(row < 0 )  row = 0 ;
				if(row >= feature_row_offset)  row = (feature_row_offset - 1) ;

				 for(int32 j = 0 ; j < num_col ; ++j)
				 {
					 col = i * num_col + j ;

					 temp = input[row * num_col + j] ;                     /* splice */

					 temp += shift_[col] ;                                 /* shift */

					 temp *= scale_[col] ;                                 /* scale */

					 output[(r - covered_feature_row_offset) * out_num_col + col] = temp ;
				}
			}
		}
	}else {		
		/* frame skipping for every two frames */
		for(int32 r = covered_feature_row_offset ; r < covered_feature_row_offset + (int32)(covered_row_size) ; r += 2)
		{
			for(int32 i = 0 ; i < splice_dim_ ; ++i)
			{
				row = r + splice_[i] ;
				if(row < 0 )  row = 0 ;
				if(row >= feature_row_offset)  row = (feature_row_offset - 1) ;

				for(int32 j = 0 ; j < num_col ; ++j)
				{
					col = i * num_col + j ;

					temp = input[row * num_col + j] ;                    /* splice */

					temp += shift_[col] ;                                /* shift */

					temp *= scale_[col] ;                                /* scale */

					output[((r - covered_feature_row_offset)>> 1) * out_num_col + col] = temp ;
				}
			}
		}
	}
}
