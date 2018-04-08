/*
 * main.c
 *
 *  Created on : 8/19/2014
 *      Author : zhiming.wang
 */

# include "asr.h"
# include "common.h"
//# include <vxWorks.h>
# include <unistd.h>
//# include <ioLib.h>
# include <stdio.h>
# include <stdlib.h>
# include <assert.h>
# include <time.h>
//# include <taskLib.h>


void copy_str(char** dst , const char* src)
{
	size_t len = strlen(src) ;
	*dst = (char*)malloc(sizeof(char) * (len + 1)) ;
	assert(NULL != *dst) ;
	memcpy((void*)(*dst) ,(const void*)src , sizeof(char) * len) ;
	(*dst)[len] ='\0' ;
}

void free_str(char** str)
{
	if(NULL != *str)
	{
		free(*str) ;
		*str = NULL ;
	}
}



int main(void)
{

	
	printf("......the ASR Engine is running......\n") ;

	/* read form file to set asr config */
	/* or manually to set asr config is also OK */
	struct ASRConfig asr_config ;
	
	FILE* g  = fopen("../project/1/model/asr.config" , "rb") ;
	if(NULL == g)
	{
		printf("error : failed to open ASR config file : asr.config") ;
		exit(-1) ;
	}
	
	char ptr_char[512] ;

	if(!feof(g))
	{
		fscanf(g , "%u" , &asr_config.cmn_window) ;
		fscanf(g , "%f" , &asr_config.acoustic_scale) ;
		fscanf(g , "%u" , &asr_config.max_active) ;
		fscanf(g , "%u" , &asr_config.min_active) ;
		fscanf(g , "%f" , &asr_config.beam) ;
		fscanf(g , "%u" , &asr_config.batch_size) ;
		fscanf(g , "%u" , &asr_config.max_utt_length) ;
		fscanf(g , "%u" , &asr_config.inter_utt_sil) ;
		fscanf(g , "%u" , &asr_config.skip_frame) ;	
		fscanf(g , "%s" , ptr_char) ;
		copy_str((char**)&asr_config.sil_phones , ptr_char) ;	
		fscanf(g , "%s" , ptr_char) ;
		copy_str((char**)&asr_config.word_exclusion , ptr_char) ;	
		fscanf(g , "%s" , ptr_char) ;
		copy_str((char**)&asr_config.hmm , ptr_char) ;	
		fscanf(g , "%s" , ptr_char) ;
		copy_str((char**)&asr_config.hclg , ptr_char) ;
		fscanf(g , "%s" , ptr_char) ;
		copy_str((char**)&asr_config.word , ptr_char) ;
		fscanf(g , "%s" , ptr_char) ;
		copy_str((char**)&asr_config.feature_transform , ptr_char) ;
		fscanf(g , "%s" , ptr_char) ;
		copy_str((char**)&asr_config.net_model , ptr_char) ;
		fscanf(g , "%s" , ptr_char) ;
		copy_str((char**)&asr_config.pdf_count , ptr_char) ;
		fscanf(g , "%s" , ptr_char) ;
		copy_str((char**)&asr_config.phone_list , ptr_char) ;
		fscanf(g , "%s" , ptr_char) ;
		copy_str((char**)&asr_config.word_list , ptr_char) ;
		fscanf(g , "%s" , ptr_char) ;
		copy_str((char**)&asr_config.syllable , ptr_char) ;
		fscanf(g , "%s" , ptr_char) ;
		copy_str((char**)&asr_config.dict , ptr_char) ;
	}
	
	fclose(g) ;
	
	/* in fact , the following two steps could be viewed as just one step when with just one speaker terminal */
	/* launch the whole asr engine , and start up one asr terminal */
	asr_launch(&asr_config) ;
	void* engine =  asr_initialize(0) ;
	
	FILE* f = fopen("./wav1_old.scp" , "rb") ;
	if(NULL == f)
	{
		printf("error : failed to open wav file : ./wav.scp") ;
		exit(-1) ;
	}

	char key[256] ;
	char wav_path[512] ;
	unsigned int length = 0 ;
	float* data = NULL ;
	short* short_data = NULL ;

	
	
	if(!feof(f))
	{
		fscanf(f , "%s" , key) ;
		fscanf(f , "%s" , wav_path) ;
	}

	while(!feof(f))
	{
		//clock_gettime(CLOCK_REALTIME,&tstart);	
		/* start up one round of speech or speaking */
		asr_start(engine) ;
		
		/* the first step : get ready of data */
		printf("%s " , wav_path) ;

		wav_read(wav_path , &data , &length) ;

		short_data = (short*)malloc(sizeof(short) * length) ;

		assert(NULL != short_data) ;
		for( unsigned int j = 0 ; j < length ; j++)  short_data[j] = (short)(data[j]) ;
		
		/* sned data into asr engine */
		asr_put_data(engine , short_data , length) ;

		/* when data is ready , tell asr engine that the process of sending data is finished */
		asr_data_end(engine) ; 

		free(data) ;  data = NULL ;
		
		free(short_data) ;  short_data = NULL ;

		/* the next utterance */
		fscanf(f , "%s" , key) ;
		fscanf(f , "%s" , wav_path) ;
		/* the second step : decode */
		//clock_gettime(CLOCK_REALTIME,&tstart);	
		while(1)
		{
			/* decoding */
			int status = asr_run(engine);

			/* get asr recognized result */
			const char* result = asr_get_result(engine);

			if(strlen(result)>0) printf("%s ", result);

			if(status)  break ;
		} /* end while loop */
		printf("\n");
		//clock_gettime(CLOCK_REALTIME,&tend);
		//tcount += (float)(tend.tv_sec-tstart.tv_sec)+(float)(tend.tv_nsec-tstart.tv_nsec)/1000000000.0;
		/* the third step : close */
		/* close one round of speech or speaking , clean up back-end buffer */
		asr_close(engine) ; 
	}
	
	/* in fact , the following two steps could be viewed as just one step when with just one speaker terminal */
 	/* shut down asr engine , and exit */
	asr_exit(engine) ;
	asr_shut_down() ;

	fclose(f) ;

	free_str((char**)&asr_config.sil_phones) ;
	free_str((char**)&asr_config.word_exclusion) ;	
	free_str((char**)&asr_config.hmm) ;
	free_str((char**)&asr_config.hclg) ;
	free_str((char**)&asr_config.word) ;
	free_str((char**)&asr_config.feature_transform) ;
	free_str((char**)&asr_config.net_model) ;
	free_str((char**)&asr_config.pdf_count) ;
	free_str((char**)&asr_config.phone_list) ;
	free_str((char**)&asr_config.word_list) ;
	free_str((char**)&asr_config.syllable) ;
	free_str((char**)&asr_config.dict) ;

	return 0 ;
}

