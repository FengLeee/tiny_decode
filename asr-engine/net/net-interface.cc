/*
 * net-interface.cc
 *
 *  Created on : 07/17/2014
 *      Author : zhiming.wang
 */

# include "net-interface.h"

NetInterface::NetInterface() : input_dim_(0) , output_dim_(0) , net_input_(NULL) , net_(NULL) , prior_(NULL)  {}

NetInterface::NetInterface(const char* transform , const char* net_model , const char* prior_pro , bool skip_frame , BaseFloat scale_factor) : input_dim_(0) , output_dim_(0) , net_input_(NULL) , net_(NULL) , prior_(NULL)
{
	initialize(transform , net_model , prior_pro , skip_frame , scale_factor) ;
}

NetInterface::~NetInterface()
{
	release() ;
}

/* initialize */
void NetInterface::initialize(const char* transform , const char* net_model , const char* prior_pro , bool skip_frame , BaseFloat scale_factor)
{
	net_input_ = (NetInput*)malloc(sizeof(NetInput)) ;
	assert(NULL != net_input_) ;
	net_input_->initialize(transform , skip_frame , scale_factor) ;

	input_dim_ = net_input_->input_dim() ;

	net_ = (Net*)malloc(sizeof(Net)) ;
	assert(NULL != net_) ;
	net_->initialize() ;
	net_->binary_read(net_model) ;

	output_dim_ = net_->output_dim() ;

	prior_ = (Prior*)malloc(sizeof(Prior)) ;
	assert(NULL != prior_) ;
	prior_->initialize(prior_pro) ;

	assert(prior_->get_dim() == output_dim_) ;
}

/* release */
void NetInterface::release()
{
	if(NULL != net_input_)
	{
		net_input_->release() ;
		free(net_input_) ;
		net_input_ = NULL ;
	}

	if(NULL != net_)
	{
		net_->release() ;
		free(net_) ;
		net_ = NULL ;
	}

	if(NULL != prior_)
	{
		prior_->release() ;
		free(prior_) ;
		prior_ = NULL ;
	}

	input_dim_ = output_dim_ = 0 ;
}

int32 NetInterface::input_dim() const
{
	return input_dim_ ;
}

int32 NetInterface::output_dim() const
{
	return output_dim_ ;
}

const Prior* NetInterface::get_prior() const
{
	return (const Prior*)prior_ ;
}

const Net* NetInterface::get_net() const
{
	return (const Net*)net_ ;
}

const NetInput* NetInterface::get_net_input() const
{
	return (const NetInput*)net_input_ ;
}

/* net forward compute */
void NetInterface::compute(const BaseFloat* input , int32 num_row , int32 num_col , BaseFloat* output , int32 out_num_row , int32 out_num_col , BaseFloat scale , int32 feature_row_offset , int32 covered_feature_row_offset , uint32 covered_row_size) const
{
	assert(num_col == input_dim_ && out_num_col == output_dim_) ;
	assert(NULL != input && NULL != output) ;

	BaseFloat* trans_feat = (BaseFloat*)malloc(sizeof(BaseFloat) * out_num_row * net_->input_dim()) ;
	assert(NULL != trans_feat) ;

	/* first , net transform */
	((const NetInput*)net_input_)->forward(input , num_row , num_col , trans_feat , out_num_row , net_->input_dim() , feature_row_offset , covered_feature_row_offset , covered_row_size) ;

	/* then , net forward propagation */
	((const Net*)net_)->forward(trans_feat , out_num_row , net_->input_dim() , output) ;

	/* at last , subtract priors from log posterior probabilities to get pseudo log likelihoods */
	((const Prior*)prior_)->subtract_log_post(output , out_num_row , out_num_col , scale) ;

	free(trans_feat) ;
	trans_feat =NULL ;
}
