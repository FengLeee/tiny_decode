/*
 * wave-reader.h
 *
 *  Created on : 6/11/2014
 *      Author : zhiming.wang
 */

# ifndef _WAVE_READER_H_
# define _WAVE_READER_H_

# include "../base/common.h"

class WaveData {
public :
	WaveData()  {}
	~WaveData()  {}

	/* read wave , and store the the actual sound data */
	int read(const char* wave , BaseFloat** data , uint32* length) ;

private :
	uint16 read_uint16(FILE* f , int32 swap) ;

	uint32 read_uint32(FILE* f , int32 swap) ;

	int read_quad_bytes(FILE* f , char* quad_bytes) ;

	int expect_quad_bytes(FILE* f , const char* expected_quad_bytes) ;

} ;

# endif /* _WAVE_READER_H_ */
