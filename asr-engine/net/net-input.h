/*
 * net-input.h
 *
 *  Created on : 07/17/2014
 *      Author : zhiming.wang
 */

# ifndef _NET_INPUT_H_
# define _NET_INPUT_H_

# include "../base/common.h"

class NetInput
{
public :
	NetInput() ;
	NetInput(const char* transform_file , bool skip_frame = false , BaseFloat scale_factor = 1.0) ;
	~NetInput() ;

	/* initialize */
	void initialize(const char* transform , bool skip_frame = false , BaseFloat scale_factor = 1.0) ;

	/* release */
	void release() ;

	bool skipping_frame() const ;

	int32 input_dim() const ;

	int32 output_dim() const ;
	
	inline int32 left_context()  const
	{
		return left_context_ ;
	}
	
	inline int32 right_context()  const
	{
		return right_context_ ;
	}
	
	/* net input transform */
	void forward(const BaseFloat* input , int32 num_row , int32 num_col , BaseFloat* output , int32 out_num_row , int32 out_num_col , int32 feature_row_offset , int32 covered_feature_row_offset , uint32 covered_row_size)  const ;

private :
	int32 input_dim_ ;
	int32 output_dim_ ;
	int32 splice_dim_ ;
	int32 left_context_ ;
	int32 right_context_ ;

	bool skip_frame_ ;
	
	int32* splice_ ;
	BaseFloat* shift_ ;
	BaseFloat* scale_ ;

} ;

# endif /* _NET_INPUT_H_ */
