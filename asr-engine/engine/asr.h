# ifndef _ASR_H_
# define _ASR_H_

# ifdef __cplusplus
extern "C"{
# endif

struct ASRConfig {
	int cmn_window ;
	float acoustic_scale ;
	int max_active ;
	int min_active ;
	float beam ;
	int batch_size ;
	int max_utt_length ;
 	int inter_utt_sil ;
	int skip_frame ;
	const char* sil_phones ;
	const char* word_exclusion ;
	const char* hmm ;
	const char* hclg ;
	const char* word ;
	const char* feature_transform ;
	const char* net_model ;
	const char* pdf_count ; 
	const char* phone_list ;
	const char* word_list ;
	const char* syllable ;
	const char* dict ;
} ;

/* launch asr based on config */
int asr_launch(const struct ASRConfig* asr_config) ;

/* shut down asr , free all resource */
int asr_shut_down() ;

/* initialize an asr object */
void* asr_initialize(int worker) ;

/* an asr object is exiting */
void asr_exit(void* obj) ;

/* start an asr object */
void asr_start(void* obj) ;

/* close an asr object */
void asr_close(void* obj) ;

/* put some data to asr object */
void asr_put_data(void* obj , short* data , int len) ;

/* when all input data is ready , tell the asr object that all data is finished */
void asr_data_end(void* obj) ;

/* when having data , call asr_run to compute the result */
int asr_run(void* obj) ;

/* when runing some time , call asr_get_result to get text result */
const char* asr_get_result(void* obj) ;

# ifdef __cplusplus
} ;
# endif

# endif
