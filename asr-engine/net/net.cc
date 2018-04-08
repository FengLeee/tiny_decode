/*
 * net.cc
 *
 *  Created on : 07/07/2014
 *      Author : zhiming.wang
 */

# include "net.h"

Net::Net() : mlp_component_(NULL) , num_layer_(0)
{
	initialize() ;
}

Net::~Net()
{
	release() ;
}

/* initialize */
void Net::initialize()
{
	mlp_component_ = NULL ;
	num_layer_ = 0 ;
}

/* release */
void Net::release()
{
	if(NULL != mlp_component_)
	{
		for(int32 i = 0 ; i < num_layer_ ; ++i)
			mlp_component_[i].release() ;

		free(mlp_component_) ;
		mlp_component_ = NULL ;
	}

	num_layer_ = 0 ;
}

/* read in neural net model */
void Net::read(const char* net_model)
{
	FILE* f = fopen(net_model , "rb") ;
	if(NULL == f)
	{
		char err[100] ;
		sprintf(err , "error : failed to open neural net file : %s" , net_model) ;
		error(err , __FILE__ , __LINE__) ;
	}

	num_layer_ = 0 ;
	char str[512] ;

	if(!feof(f))  fscanf(f , "%s" , str) ;

	while(!feof(f))
	{
		if(0 == num_layer_ || NULL == mlp_component_)
			mlp_component_ = (MLPComponent*)malloc(sizeof(MLPComponent)) ;
		else
			mlp_component_ = (MLPComponent*)realloc(mlp_component_ , sizeof(MLPComponent) * (num_layer_ + 1)) ;
		if(NULL == mlp_component_)  error("no memory to be allocated" , __FILE__ , __LINE__) ;

		mlp_component_[num_layer_++].read(f) ;

		fscanf(f , "%s" , str) ;
	}

	fclose(f) ;
}

/* read in binary neural net model */
void Net::binary_read(const char* net_model)
{
	FILE* f = fopen(net_model , "rb") ;
	if(NULL == f)
	{
		char err[100] ;
		sprintf(err , "error : failed to open binary neural net file : %s" , net_model) ;
		error(err , __FILE__ , __LINE__) ;
	}

	num_layer_ = 0 ;
	char str[512] ;

	if(!feof(f))
	{
		fread(str , 1 , 2 , f) ;
		fscanf(f , "%s" , str) ;  /* token */
	}

	while(!feof(f))
	{
		if(0 == num_layer_ || NULL == mlp_component_)
			mlp_component_ = (MLPComponent*)malloc(sizeof(MLPComponent)) ;
		else
			mlp_component_ = (MLPComponent*)realloc(mlp_component_ , sizeof(MLPComponent) * (num_layer_ + 1)) ;
		if(NULL == mlp_component_)  error("no memory to be allocated" , __FILE__ , __LINE__) ;

		mlp_component_[num_layer_++].binary_read(f) ;

		fscanf(f , "%s" , str) ;  /* token */
	}

	fclose(f) ;
}

int32 Net::number_of_layer() const
{
	return num_layer_ ;
}

int32 Net::input_dim() const
{
	return (num_layer_ > 0) ? mlp_component_[0].input_dim() : -1 ;
}

int32 Net::output_dim() const
{
	return (num_layer_ > 0) ? mlp_component_[num_layer_ - 1].output_dim() : -1 ;
}

/* net forward propagation */
void Net::forward(const BaseFloat* input , int32 num_row , int32 num_col , BaseFloat* output) const
{
	assert(num_col == input_dim()) ;
	assert(NULL != output) ;

	if(0 == num_layer_)
	{
		memcpy((void*)output , (const void*)input , sizeof(BaseFloat) * num_row * num_col) ;
		return ;
	}

	BaseFloat* in = NULL ;

	AlignedAlloc(&in , sizeof(BaseFloat) * num_row * num_col) ;

	/* copy */
	memcpy((void*)in , (const void*)input , sizeof(BaseFloat) * num_row * num_col) ;

	if(1 == num_layer_)
	{
		mlp_component_[0].forward(in , num_row , num_col , output) ;
		AlignedFree(&in) ;
		return ;
	}

	/* need at least 2 auxiliary buffers */
	BaseFloat* propagate_buffer[2] ;
	propagate_buffer[0] = propagate_buffer[1] = NULL ;

	int32 layer = 0 ;

	AlignedAlloc(&propagate_buffer[layer % 2] , sizeof(BaseFloat) * num_row * mlp_component_[layer].output_dim()) ;

	/* net forward propagation */
	mlp_component_[layer].forward(in , num_row , num_col , propagate_buffer[layer % 2]) ;

	for(layer++ ; layer <= (num_layer_ - 2) ; layer++)
	{
		if(NULL != propagate_buffer[layer % 2])  AlignedFree(&propagate_buffer[layer % 2]) ;

		AlignedAlloc(&propagate_buffer[layer % 2] , sizeof(BaseFloat) * num_row * mlp_component_[layer].output_dim()) ;

		mlp_component_[layer].forward(propagate_buffer[(layer - 1) % 2] , num_row , mlp_component_[layer].input_dim() , propagate_buffer[layer % 2]) ;
	}

	mlp_component_[layer].softmax(propagate_buffer[(layer - 1) % 2] , num_row , mlp_component_[layer].input_dim() , output) ;

	for(int32 i = 0 ; i < 2 ; i++)
		if(NULL != propagate_buffer[i])  AlignedFree(&propagate_buffer[i]) ;

	AlignedFree(&in) ;
}
