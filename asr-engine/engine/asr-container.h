/*
 * asr-container.h
 *
 *  Created on : 8/20/2014
 *      Author : zhiming.wang
 */

# ifndef _ASR_CONTAINER_H_
# define _ASR_CONTAINER_H_

# include "../hmm/hmm.h"
# include "../util/word-symbol.h"
# include "../util/wfst.h"
# include "../net/net-interface.h"
# include "../decode/online-faster-decoder.h"
# include "asr.h"
# include "yt_asr_option.h"

class ASRContainer
{
public :
	ASRContainer() : hmm_(NULL) , fst_(NULL) , word_symbol_(NULL) , net_interface_(NULL) , fft_(NULL) ,  \
	ptr_phone_(NULL) , ptr_phone_buffer_(NULL) , ptr_word_(NULL) , ptr_word_buffer_(NULL) , ptr_syllable_(NULL) , ptr_syllable_buffer_(NULL) ,  ptr_dict_(NULL) , ptr_dict_buffer_(NULL)
	{
	}

	ASRContainer(const struct ASRConfig* asr_config) : hmm_(NULL) , fst_(NULL) , word_symbol_(NULL) , net_interface_(NULL) , fft_(NULL) ,  \
	ptr_phone_(NULL) , ptr_phone_buffer_(NULL) , ptr_word_(NULL) , ptr_word_buffer_(NULL) , ptr_syllable_(NULL) , ptr_syllable_buffer_(NULL) ,  ptr_dict_(NULL) , ptr_dict_buffer_(NULL)
	{
		initialize(asr_config) ;
	}
	
	~ASRContainer()
	{
		release() ;
	}

	/* initialize */
	void initialize(const struct ASRConfig* asr_config) ;

	/* release */
	void release() ;

	inline const HMM* get_hmm()  const
	{
		return (const HMM*)hmm_ ;
	}

	inline const WFST* get_fst()  const
	{
		return (const WFST*)fst_ ;
	}

	inline const WordSymbol* get_word_symbol()  const
	{
		return (const WordSymbol*)word_symbol_ ;
	}

	inline const NetInterface* get_net_interface()  const
	{
		return (const NetInterface*)net_interface_ ;
	}
	
	inline const FFT* get_fft()  const
	{
		return (const FFT*)fft_ ;
	}

	inline const VectorI& get_sil_phones()  const
	{
		return (const VectorI&)sil_phones_ ;
	}

	const ASRConfig& get_asr_config()  const
	{
		return asr_config_ ;
	}

public :
	BaseFloat acoustic_scale_ ;
	int32 cmn_window_ ;
	FilterBankOptions filter_bank_options_ ;
	FasterDecoderOptions decoder_config_ ;

	VectorI word_exclusion_ ;

private :
	HMM* hmm_ ;
	WFST* fst_ ;
	WordSymbol* word_symbol_ ;

	NetInterface* net_interface_ ;
	
	FFT* fft_ ;

	VectorI sil_phones_ ;
	
	ASRConfig asr_config_ ;

public :
	char** ptr_phone_ ;
  	char* ptr_phone_buffer_ ;
 	unsigned int num_phone_entry_ ;

	char** ptr_word_ ;
  	char* ptr_word_buffer_ ;
 	unsigned int num_word_entry_ ;

	char** ptr_syllable_ ;
  	char* ptr_syllable_buffer_ ;
 	unsigned int num_syllable_entry_ ;

	char** ptr_dict_ ;
  	char* ptr_dict_buffer_ ;
 	unsigned int num_dict_entry_ ;
	
} ;

# endif /* _ASR_CONTAINER_H_ */
