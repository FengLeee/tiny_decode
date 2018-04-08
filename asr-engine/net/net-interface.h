/*
 * net-interface.h
 *
 *  Created on : 07/17/2014
 *      Author : zhiming.wang
 */

# ifndef _NET_INTERFACE_H_
# define _NET_INTERFACE_H_

# include "net.h"
# include "prior.h"
# include "net-input.h"

class NetInterface
{
public :
	NetInterface() ;
	NetInterface(const char* transform , const char* net_model , const char* prior_pro , bool skip_frame = false , BaseFloat scale_factor = 1.0) ;
	~NetInterface() ;

	/* initialize */
	void initialize(const char* transform , const char* net_model , const char* prior_pro , bool skip_frame = false , BaseFloat scale_factor = 1.0) ;

	/* release */
	void release() ;

	int32 input_dim() const ;

	int32 output_dim() const ;

	const Prior* get_prior() const ;

	const Net* get_net() const ;

	const NetInput* get_net_input() const ;
		
	/* net forward compute */
	void compute(const BaseFloat* input , int32 num_row , int32 num_col , BaseFloat* output , int32 out_num_row , int32 out_num_col , BaseFloat scale , int32 feature_row_offset , int32 covered_feature_row_offset , uint32 covered_row_size) const ;

private :
	int32 input_dim_ ;
	int32 output_dim_ ;

	NetInput* net_input_ ;
	Net* net_ ;
	Prior* prior_ ;

} ;

# endif /* _NET_INTERFACE_H_ */
