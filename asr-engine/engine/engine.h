/*
 * engine.h
 *
 *  Created on : 08/29/2014
 *      Author : zhiming.wang
 */

# ifndef _ENGINE_H_
# define _ENGINE_H_

# include "asr-container.h"
//# include <vxWorks.h>
# include <stdio.h> 
# include "../util/cstring.h"
# include "../util/vector.h"

class ASREngine
{
public :
	inline ASREngine() : filter_bank_input_(NULL) , asr_container_(NULL)  {}

	inline ASREngine(const ASRContainer* asr_container) : filter_bank_input_(NULL) , asr_container_(asr_container)
	{
		filter_bank_input_ = (FilterBankInput*)malloc(sizeof(FilterBankInput)) ;
		if(NULL == filter_bank_input_)  error("error to allocate memory for filter bank input" , __FILE__ , __LINE__) ;

		mutex_.initialize() ;
	}

	~ASREngine() ;

	/* initialize */
	void Initialize(const ASRContainer* asr_container) ;

	/* release */
	void Release() ;

	void Start() ;

	void Close() ;

	int32 SendData(int16* data , int32 length) ;

	int32 StopSendData() ;

	bool Decode() ;
	
	void RecordPartialResult(const VectorI& phone_vec , const VectorI& word_vec , bool utt_end) ;
 	
	const char* GetPartialResult() ;
	
public :
	int32 worker_ ;

private :
	DNNDecodableInterface decodable_ ;
	OnlineFasterDecoder	decoder_ ;
	FilterBankInput* filter_bank_input_ ;

	const ASRContainer* asr_container_ ;

	Mutex mutex_ ;

	String string_buffer_ ;
	String string_buffer_forward_ ;
	VectorI phone_vec_ ;
	VectorI phone_stat_ ;
	
} ;

# endif /* _ENGINE_H_ */
