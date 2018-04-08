/*
 * wfst.h
 *
 *  Created on : 7/3/2014
 *      Author : zhiming.wang
 */

# ifndef _WFST_H_
# define _WFST_H_

# include "../base/common.h"

struct Arc_ {
	int32 input_label ;
	int32 output_label ;
	BaseFloat arc_weight ;
	int32 next_state ;

	Arc_() : input_label(0) , output_label(0) , arc_weight(0.0) , next_state(0)  {}

} ;

struct StateNode {
	BaseFloat weight ;  /* e.g. , final weight */
	int64 num_arc ;
	Arc_* arc ;

	StateNode() : weight(0.0) , num_arc(0) , arc(NULL)  {}

} ;

class WFST {
public :
	WFST() ;
	~WFST() ;

	/* initialize */
	void initialize() ;

	/* release */
	void release() ;

	/* read wfst file */
	void read(const char* wfst_file) ;

	const StateNode& state(int64 state_index) const ;

	int64 number_of_state_node() const ;

	int64 get_start_state() const ;

private :
	StateNode* graph ;
	int64 num_state_node ;
	int64 start_state ;

} ;

# endif /* _WFST_H_ */
