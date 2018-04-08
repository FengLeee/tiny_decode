# ifndef __YT_ASR_OPTION_HEADER__
# define __YT_ASR_OPTION_HEADER__


/*--------------------------------------------------------------------
 |YT_LoadOneTextFileWithKey(...)
 |PURPOSE
 | This API intends to load a text file into memory. Also it converts
 | the text buffer into line array.
 |
 |INPUT
 | charKey: key for encryption
 | strTextFile: target text file
 | 
 |OUTPUT
 | ppBuffer: pointer to buffer for holding text 
 | pLineNumber: pointer to line number
 |
 |RETURN VALUE
 | If successful, it returns the corresponding pointer array;
 | else it reurns NULL
 |
 |CODING
 | Gui-Lin Chen
 *-------------------------------------------------------------------*/
char **YT_LoadOneTextFileWithKey(unsigned char charKey,//[in]
				     char *strTextFile,//[in]
				     char **ppBuffer,//[out]
				     unsigned int *pLineNumber);//out

int YT_ASR_CheckResults(char *strWaveFile_IN,
			   unsigned int nAlignIndex,
			   char *strWaveFile_OUT,
			   char *strAlignFile_Phone, 
			   char *strAlignFile_Word, 
			   char *strDictFile_Phone, 
			   char *strDictFile_Word);

int YT_ASR_CheckResults_V2(char *strDictFile_Phone, 
			      char *strDictFile_Word,
			      char *strWaveFile_IN,
			      char *strWaveFile_OUT,														
			      char *strAlignPhone_IN,
			      char *strAlignPhone_OUT);	

void YT_GetFields_NEW(unsigned int MAX_FIELD_NUMBER,
			 char *strLine,//[in]
			 char **ppField,//[out]
			 unsigned int *pFieldNumber);//[out]

int YT_StringBinarySearchWithLength(char *strToFind, char *strArray[], unsigned int nStart_IN, unsigned int nEnd_IN, int *pClosed,unsigned int nLenToFind);

/*----------------------------------------------------------------
 |YT_ASR_CheckRecResult_NEW(...)
 |PURPOSE
 |	This API intends to conduct post-processing based on 
 |	input waveform/phone_sequence/word_sequence. 
 |
 |INPUT
 |	strWaveFile_IN: the input wave file
 |	strRecResult_Phone_IN: the input phone sequence for recognition result
 |	strRecResult_Word_IN: the input word sequence for recognition result
 |
 |	ptrArray_TTS_ASR_TAB: string array for table mapping from TTS to ASR
 |	nEntryNumber_TTS_ASR_TAB: entry number of the mapping table
 |
 |	ptrArray_SWAP: string array for lexicon mapping phone sequence to word
 |	nEntryNumber_SWAP: entry number of the lexicon
 |
 |	ptrArray_PhoneDict: string array for phone list
 |	nEntryNumber_PhoneDict: entry number of phone list
 |
 |	ptrArray_WordDict: string array for word list
 |	nEntryNumber_WordDict: entry number of word list
 |
 |	strWaveFile_OUT: the output wave file after normalization
 |	strRecResult_Phone_OUT: the output phone sequence for recognition result
 |	strRecResult_Word_OUT: the output word sequence for recognition result
 *---------------------------------------------------------------*/
int YT_ASR_CheckRecResult_NEW(/* char *strWaveFile_IN, */
							  char *strRecResult_Phone_IN,
							  char *strRecResult_Word_IN,
							  
							  char **ptrArray_TTS_ASR_TAB, 
							  unsigned int nEntryNumber_TTS_ASR_TAB,

							  char **ptrArray_SWAP,
							  unsigned int nEntryNumber_SWAP,
							  
							  char **ptrArray_PhoneDict,
							  unsigned int nEntryNumber_PhoneDict,
							  
							  char **ptrArray_WordDict,
							  unsigned int nEntryNumber_WordDict,

							  /* char *strWaveFile_OUT, */
							  char *strRecResult_Phone_OUT,
							  char *strRecResult_Word_OUT);

// added by FHL
int YT_ASR_CheckRecResult_NEW_V2(/* char *strWaveFile_IN, */
							  char *strRecResult_Phone_IN,
							  char *strRecResult_Word_IN,

							  char **ptrArray_TTS_ASR_TAB,
							  unsigned int nEntryNumber_TTS_ASR_TAB,

							  char **ptrArray_SWAP,
							  unsigned int nEntryNumber_SWAP,

							  char **ptrArray_PhoneDict,
							  unsigned int nEntryNumber_PhoneDict,

							  char **ptrArray_WordDict,
							  unsigned int nEntryNumber_WordDict,

							  /* char *strWaveFile_OUT, */
							  char *strRecResult_Phone_OUT,
							  char *strRecResult_Word_OUT);

# endif
