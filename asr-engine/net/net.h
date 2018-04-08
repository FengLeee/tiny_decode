/*
 * net.h
 *
 *  Created on : 07/07/2014
 *      Author : zhiming.wang
 */

# ifndef _NET_H_
# define _NET_H_

# include "component.h"

class Net {
public :
	Net() ;
	~Net() ;

	/* initialize */
	void initialize() ;

	/* release */
	void release() ;

	/* read in neural net model */
	void read(const char* net_model) ;

	/* read in binary neural net model */
	void binary_read(const char* net_model) ;

	int32 number_of_layer() const ;

	int32 input_dim() const ;

	int32 output_dim() const ;

	/* net forward propagation */
	void forward(const BaseFloat* input , int32 num_row , int32 num_col , BaseFloat* output) const ;

private :
	MLPComponent* mlp_component_ ;
	int32 num_layer_ ;

} ;

# endif /* _NET_H_ */
