/*
 * asr.cc
 *
 *  Created on : 09/02/2014
 *      Author : zhiming.wang
 */

# include "engine.h"

static ASRContainer* asr_container_ = NULL ;

/* launch asr based on config */
int asr_launch(const struct ASRConfig* asr_config)
{
	asr_container_ = new ASRContainer ;
		
	assert(NULL != asr_container_) ;
		
	asr_container_->initialize(asr_config) ;

	printf("[ASR Initialize]---------------->OK \n") ;

	return 0 ;
}

/* shut down asr , free all resource */
int asr_shut_down()
{
	if(NULL != asr_container_)
	{
		asr_container_->release() ;
		
		delete asr_container_ ;
		
		asr_container_ = NULL ;
	}

	return 0;
}

/* initialize an asr object */
void* asr_initialize(int worker)
{
	ASREngine* engine = new ASREngine(asr_container_) ;
	
	assert(NULL != engine) ;

	engine->worker_ = worker ;
	
	return static_cast<void*>(engine) ;
}

/* an asr object is exiting */
void asr_exit(void* obj) 
{
	ASREngine* engine = static_cast<ASREngine*>(obj) ;
	
	if(NULL != engine)
	{
		engine->Release() ;

		delete engine ;
		
		engine = NULL ;
	}
}

/* start an asr object */
void asr_start(void* obj)
{
	ASREngine* engine = static_cast<ASREngine*>(obj) ;
	
	engine->Start() ;
}

/* close an asr object */
void asr_close(void* obj)
{
	ASREngine* engine = static_cast<ASREngine*>(obj) ;

	engine->Close() ;
}

/* put some data to asr object */
void asr_put_data(void* obj , short* data , int len)
{
	ASREngine* engine = static_cast<ASREngine*>(obj) ;

	engine->SendData(data , len) ;
}

/* when all input data is ready , tell the asr object that all data is finished */
void asr_data_end(void* obj)
{
	ASREngine* engine = static_cast<ASREngine*>(obj) ;
	
	engine->StopSendData() ;
}

/* when having data , call asr_run to compute the result */
int asr_run(void* obj) 
{
	ASREngine* engine = static_cast<ASREngine*>(obj) ;

	return engine->Decode() ;
}

/* when runing some time , call asr_get_result to get text result */
const char* asr_get_result(void* obj)
{
	ASREngine* engine = static_cast<ASREngine*>(obj) ;

	return engine->GetPartialResult() ;
}
