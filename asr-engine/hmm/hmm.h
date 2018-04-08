/*
 * hmm.h
 *
 *  Created on : 6/21/2014
 *      Author : zhiming.wang
 */

# ifndef _HMM_H_
# define _HMM_H_

# include "../base/common.h"

struct HMMQuad {
	uint32 phone ;
    uint32 hmm_state ;
    uint32 pdf ;
    uint32 num_trans ;

    HMMQuad() : phone(0) , hmm_state(0) , pdf(0) , num_trans(0) {}

} ;

class HMM {
public :
	HMM() ;
	~HMM() ;

	/* initialize */
	void initialize() ;

	/* release */
	void release() ;

	/* read hmm file */
	void read(const char* hmm_file) ;

	uint32 num_pdfs() const ;

	uint32 num_transitions() const ;

	/* transition id -> state */
	uint32 transition2state(uint32 id) const ;

	/* transition id -> pdf */
	uint32 transition2pdf(uint32 id) const ;

	/* transition id -> phone */
	uint32 transition2phone(uint32 id) const ;

private :
	HMMQuad* hmm_quad_ ;
	uint32 num_hmm_quad_ ;

	uint32* state2id_ ;
	uint32 num_state2id_ ;

	uint32* id2state_ ;
	uint32 num_id2state_ ;

	uint32 num_pdfs_ ;

} ;

# endif /* _HMM_H_ */
