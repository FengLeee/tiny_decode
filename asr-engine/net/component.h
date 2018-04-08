/*
 * component.h
 *
 *  Created on : 07/07/2014
 *      Author : zhiming.wang
 */

# ifndef _COMPONENT_H_
# define _COMPONENT_H_

# include "net-base.h"

class MLPComponent
{
public :
	MLPComponent() ;
	~MLPComponent() ;

	/* initialize */
	void initialize() ;

	/* release */
	void release() ;

	/* read in neural net model */
	void read(FILE* f) ;

	/* read in binary neural net model */
	void binary_read(FILE* f) ;

	int32 input_dim() const ;

	int32 output_dim() const ;

	/* forward propagation , with ReLU as activation function */
	void forward(const BaseFloat* input , int32 num_row , int32 num_col , BaseFloat* output) const ;

	/* apply soft max regression */
	void softmax(const BaseFloat* input , int32 num_row , int32 num_col , BaseFloat* output) const ;

private :
	int32 input_dim_ ;
	int32 output_dim_ ;
	BaseFloat* weight_ ;
	BaseFloat* bias_ ;

} ;

# endif /* _COMPONENT_H_ */
