# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <strings.h>
# include <assert.h>

//#include "./common/yt_process_wave.h"
#include "yt_asr_option.h"

// parse the wave header....
void YT_ParseWaveHeader(unsigned int *pChannelNumber,//[out], the channel number,
					 unsigned int *pSamplingRatio,//[out], the sampling ratio
					 unsigned int *pBitsPerSample,//[out], the bit number per sample
					 unsigned char *WIN_WAVEHEADER,//[in]
					 unsigned int *pDataLength)//[out], pointer to data length
{
	// the wave header: 44bytes

	// byte 0  - byte 3 : RIFF
	// byte 4  - byte 7 : file length
	// byte 8  - byte 11: WAVE
	// byte 12 - byte 15: fmt whitespace
	// byte 16 - byte 19: transition 
	// byte 20 - byte 21: format
	// byte 22 - byte 23: channel number
	// byte 24 - byte 27: sampling ratio
    	// byte 28 - byte 31: transfer ratio
   	// byte 32 - byte 33: adjusting number
	// byte 34 - byte 35: bits per sample, used in quantization...
	// byte 36 - byte 39: data
	// byte 40 - byte 43: speech length, it equals to file length - 44

	// under windows, lower-order byte precedes high-order byte...

	unsigned int nChannelNumber=1, nSamplingRatio=8096, nBitsPerSample=8, nTemp;
    	unsigned char charHigh, charLow;
	unsigned int nDataLength;

	//get the channel number
	charHigh = WIN_WAVEHEADER[23];
    	nChannelNumber = charHigh; nChannelNumber <<=8;
	charLow = WIN_WAVEHEADER[22];
    	nChannelNumber += charLow;


	//get the sampling ratio
	charHigh = WIN_WAVEHEADER[27];
   	nSamplingRatio = charHigh; nSamplingRatio <<=8;

	charLow = WIN_WAVEHEADER[26]; 
    	nSamplingRatio += charLow;

	nSamplingRatio <<= 16;

    	charHigh = WIN_WAVEHEADER[25];
	nTemp = charHigh; nTemp<<=8;
    	charLow = WIN_WAVEHEADER[24];
	nTemp += charLow;

	nSamplingRatio += nTemp;

	// get bits per sample...
    	charHigh = WIN_WAVEHEADER[35];
    	nBitsPerSample = charHigh; nBitsPerSample <<=8;
    	charLow = WIN_WAVEHEADER[34];
   	nBitsPerSample += charLow;	





	*pChannelNumber = nChannelNumber;
	*pSamplingRatio = nSamplingRatio;
	*pBitsPerSample = nBitsPerSample;


	//we need to get data length...
	charHigh = WIN_WAVEHEADER[43];
	nDataLength = charHigh;
	
	charHigh = WIN_WAVEHEADER[42];
	nDataLength <<= 8; nDataLength |= charHigh;

	charHigh = WIN_WAVEHEADER[41];
	nDataLength <<= 8; nDataLength |= charHigh;

	charHigh = WIN_WAVEHEADER[40];
	nDataLength <<= 8; nDataLength |= charHigh;

	*pDataLength = nDataLength;
}

int TEMP_AmplifyWaveformOnly(char *strWaveFile_IN,char *strWaveFile_OUT)					   
					   
{
	FILE *fp, *fpOut;

	unsigned int Fs = 16000;
	unsigned int nChannelNumber, nSamplingRate, nBitsPerSample;
	unsigned char pWaveHeader[44];
	unsigned int nLen;

	int nFileNo;
	unsigned int nFileLen;
	unsigned int i;
	short *pSpeech_IN, nShort, nMax;
	unsigned int nDataLength;

	unsigned int FRAME_SHIFT = 80, nTimes, nRemainder;
	double  dTemp;


	fp = fopen(strWaveFile_IN,"rb");
	if(NULL == fp) return -1;
	
	fpOut = fopen(strWaveFile_OUT,"wb");
	if(NULL == fpOut)
	{
		fclose(fp);
		return -1;
	}


	nFileNo = fileno(fp);
	fseek(fp,0L,SEEK_END);
	nFileLen = ftell(fp);
	fseek(fp,0L,SEEK_SET);
	
	i = 0;
	nLen = strlen(strWaveFile_IN);
	if(nLen > 4)
	{ 
	#ifdef _MSC_VER
		if(0 == stricmp(&strWaveFile_IN[nLen-4],".wav"))
		{
			i = 44;
			fread(pWaveHeader,1,44,fp);

			//parse wave header...
			YT_ParseWaveHeader(&nChannelNumber,//[out], the channel number,
							 &nSamplingRate,//[out], the sampling ratio
							 &nBitsPerSample,//[out], the bit number per sample
							 pWaveHeader,//[in], the wave header: 44bytes....
							 &nDataLength);
			Fs = nSamplingRate;
		}
	#else
		if(0 == strcasecmp(&strWaveFile_IN[nLen-4],".wav"))
		{
			i = 44;
			fread(pWaveHeader,1,44,fp);

			//parse wave header...
			YT_ParseWaveHeader(&nChannelNumber,//[out], the channel number,
							 &nSamplingRate,//[out], the sampling ratio
							 &nBitsPerSample,//[out], the bit number per sample
							 pWaveHeader,//[in], the wave header: 44bytes....
							 &nDataLength);
			Fs = nSamplingRate;
		}
	#endif
	}

	pSpeech_IN = (short *)malloc(nFileLen+1000);
	fread(pSpeech_IN, sizeof(short),nDataLength/2,fp);
	fclose(fp);

	nTimes = nDataLength / (FRAME_SHIFT * 2);
	nRemainder = nDataLength % (FRAME_SHIFT * 2);


	nMax = 0;
	for(i=0; i<nDataLength/2; i++)
	{
		nShort = pSpeech_IN[i];
		if(nShort > 0)
		{
			if(nShort > nMax) nMax = nShort;
		}
		else
		{
			if((0 - nShort) > nMax) nMax = (0 - nShort);
		}
	}

	for(i=0; i<nDataLength/2; i++)
	{
		nShort = pSpeech_IN[i];
		if(nMax > 0)
		{
			dTemp = (nShort + 0.0)/(nMax+0.0);
			dTemp *= 32000.0; //30000.0
			//dTemp *= 15000.0;
			nShort = (short)dTemp;
		}

		pSpeech_IN[i] = nShort;

	}


	fwrite(pWaveHeader,1,44,fpOut);
	fwrite(pSpeech_IN,sizeof(short),nDataLength/2,fpOut);

	free(pSpeech_IN);
	fclose(fpOut);

	return 0;
}

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
							   unsigned int *pLineNumber)//out, 
{
	FILE *fp;
	int fileNo;
	char *pBuffer;
	unsigned int nFileLength;

    	unsigned int i=0,nLineNumber=0;
	unsigned int nLineLen=0;
    	unsigned int  nLineIndex=0;

	char **ptrArray;

	unsigned char charOne, charTwo, charThree;


	
	//load the input file into memory...
	*pLineNumber=0;
	fp = fopen(strTextFile,"rb");
     	if(NULL == fp)return NULL;


      	fileNo = fileno(fp);

	fseek(fp, 0L, SEEK_END);
      	nFileLength = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	  
	  
	pBuffer=(char *) malloc((nFileLength+500));
      	fread(pBuffer,1,nFileLength,fp);
      	fclose(fp);

	if(nFileLength&&'\n' != *(pBuffer+nFileLength-1))pBuffer[nFileLength++]='\n';





	i =0;

      	nLineNumber = 0;
      	nLineLen = 0;
	while(i<nFileLength)
	{
         	charOne=*(pBuffer+i);


		if(0x09 == charOne)
		{
			*(pBuffer+i) = 0x20;
			charOne = 0x20;
		}

		if(0x0d == charOne && '\n' != *(pBuffer+i+1))
		{
			charOne = '\n';
			*(pBuffer+i) = charOne;
		}

		 if('\n' == charOne )
		 {
		     nLineNumber++;
			 nLineLen=0;
		 }
		 else nLineLen++;
		 i++;
	}
     
	if(0==nLineNumber)
	{
	   free(pBuffer);
	   return NULL;
	}



    	ptrArray = (char **)malloc((nLineNumber+2)*sizeof(char *));




	// parse the  buffer........
	nLineIndex=0;
	i=0;
	nLineLen=0;

	while(i < nFileLength)
	{
		charOne = *(pBuffer+i);

		if('\n' == charOne)
		{
			*(pBuffer+i) = '\0';
			ptrArray[nLineIndex] = (char *)(pBuffer+i-nLineLen);

			if(*ptrArray[nLineIndex]) nLineIndex++; // avoid the empty line....

			nLineLen=0;
		}
		else
		{
			if(charOne < 0x20) *(pBuffer+i) = '\0';
			nLineLen ++;
		}

		i++;
	}


	

	nLineNumber = nLineIndex;
	*pLineNumber = nLineNumber;
      	*ppBuffer = pBuffer;


	  charOne = ptrArray[0][0];
	  charTwo = ptrArray[0][1];
	  charThree = ptrArray[0][2];

	  if(0xEF == charOne && 0xBB == charTwo && 0xBF == charThree)
	  {
			ptrArray[0] = &ptrArray[0][3]; //UTF8 header
	  }



      return ptrArray;
}


void YT_GetFields_NEW(unsigned int MAX_FIELD_NUMBER,
			 char *strLine,//[in]
			 char **ppField,//[out]
			 unsigned int *pFieldNumber)//[out]
{

 register unsigned int i, nFieldIndex, nFieldLen, nResLen;
 unsigned char charOne;


 i = 0;
 nResLen = 0;
 nFieldIndex = 0;
 nFieldLen = 0;

 while(1)
 {
  charOne = strLine[i];

  if(charOne < 0x21)
  {
   strLine[i] = '\0';

   if(nFieldLen)
   {
    ppField[nFieldIndex] = strLine + i - nFieldLen;
    
    if(nFieldIndex < MAX_FIELD_NUMBER)nFieldIndex ++;
   }

   nFieldLen = 0;
  }
  else nFieldLen ++;



  if('\0' == charOne) break;
  i++;
 }


 *pFieldNumber = nFieldIndex;

}

int YT_strncmp(char *strStr1, char *strStr2, unsigned int nLenToCompare)
{
	unsigned char charOne, charTwo;
	unsigned int i=0;
	while(i<nLenToCompare)
	{
           charOne=strStr1[i];
	   charTwo=strStr2[i];


	   if(charOne>charTwo) return 1;
	   else
	   {
	      if(charOne<charTwo) return -1;          
	   }

	   if('\0'==charOne||'\0'==charTwo)break;
	   i++;
	}

   	return 0;
}

int YT_StringBinarySearchWithLength(char *strToFind, char *strArray[], unsigned int nStart_IN, unsigned int nEnd_IN, int *pClosed,unsigned int nLenToFind)
{
  int nIndex = -1; 
  int nResult = 0;
  int nStart = nStart_IN;
  int nMid = 0;
  char *pTemp;
  unsigned char charBackup;

  int nEnd = nEnd_IN - 1;
  if (nEnd<0)return  -1;
  do
	{              
		  nMid = (nStart + nEnd)>>1;
		  pTemp = strArray[nMid];

		  //avoid writing pTemp....
          
		  nResult = YT_strncmp(pTemp,strToFind,nLenToFind);
		  if(0 == nResult)
		  {
			charBackup = *(pTemp+nLenToFind);//pTemp is one element in the string set...
			if(charBackup>0x20) nResult = 1;
		  }

		  if(!nResult)
		  {
			   nIndex = nMid;
			   *pClosed = nMid;
			   return nIndex;
		  }
		  else
		  {
			  if(nResult > 0) nEnd = nMid - 1; // the middle is greater, move backward
			  else  nStart = nMid + 1;// the middle word is less, move forward
		  }

   }while(nStart <= nEnd);
  	
   if(nEnd < 0) nEnd = 0;
   *pClosed = nEnd;

   return nIndex;
}

int YT_StringBinarySearch(char *strToFind, char *strArray[], unsigned int nStart_IN, unsigned int nEnd_IN, int *pClosed)
{

  int nIndex = -1; 
  int nResult = 0;
  int nStart = nStart_IN;
  int nMid = 0;
  int nEnd = nEnd_IN - 1;
  if (nEnd < 0)return -1;

  do
	{              
		  nMid = (nStart + nEnd)>>1;
		  nResult = strcmp(strArray[nMid],strToFind);
		  if(! nResult)
		  {
			   nIndex = nMid;
			   *pClosed = nMid;
			   return nIndex;
		  }
		  else
		  {
			  if(nResult > 0) nEnd = nMid-1; /*the middle is greater, move backward*/
			  else  nStart = nMid + 1;/*the middle word is less, move forward*/
		  }
	  }while(nStart <= nEnd);
   
	if(nEnd < 0)nEnd = 0;
	*pClosed = nEnd;

  return nIndex;
}

int YT_ASR_CheckResults(char *strWaveFile_IN,
			    unsigned int nAlignIndex,
			    char *strWaveFile_OUT,
			    char *strAlignFile_Phone, 
			    char *strAlignFile_Word, 
			    char *strDictFile_Phone, 
			    char *strDictFile_Word)
{
	char **ptrArray_AlignPhone, *pBuffer_AlignPhone;
	unsigned int nEntryNumber_AlignPhone;


	char **ptrArray_AlignWord, *pBuffer_AlignWord;
	unsigned int nEntryNumber_AlignWord;


	char **ptrArray_PhoneDict, *pBuffer_PhoneDict;
	unsigned int nEntryNumber_PhoneDict;


	char **ptrArray_WordDict, *pBuffer_WordDict;
	unsigned int nEntryNumber_WordDict;


	char strEntry[3000];
	char *ppField[1000];
	unsigned int nFieldNumber;

	unsigned int nStartFrameIndex = 0, nEndFrameIndex = 0;
	unsigned int i,nShift;

	char strPhone_P[100],strPhone[100],strPhone_N[100],strPhone_N_N[100];
	unsigned int nPhoneID_P, nPhoneID, nPhoneID_N,nPhoneID_N_N;
	unsigned int nFrameNumber, nFrameNumber_N;
	unsigned char charOne, charFlag;


	TEMP_AmplifyWaveformOnly(strWaveFile_IN,strWaveFile_OUT);

	ptrArray_AlignPhone = YT_LoadOneTextFileWithKey(0,strAlignFile_Phone,&pBuffer_AlignPhone, &nEntryNumber_AlignPhone);
	assert(NULL != ptrArray_AlignPhone);


	ptrArray_AlignWord = YT_LoadOneTextFileWithKey(0,strAlignFile_Word,&pBuffer_AlignWord, &nEntryNumber_AlignWord);
	assert(NULL != ptrArray_AlignWord);


	ptrArray_PhoneDict = YT_LoadOneTextFileWithKey(0,strDictFile_Phone,&pBuffer_PhoneDict, &nEntryNumber_PhoneDict);
	assert(NULL != ptrArray_PhoneDict);


	ptrArray_WordDict = YT_LoadOneTextFileWithKey(0,strDictFile_Word,&pBuffer_WordDict, &nEntryNumber_WordDict);
	assert(NULL != ptrArray_WordDict);

	charFlag = 'N';
	strcpy(strEntry,ptrArray_AlignPhone[nAlignIndex]);
	YT_GetFields_NEW(1000,strEntry,ppField,&nFieldNumber);
	i = 1;
	while(i < nFieldNumber)
	{
		nShift = 0;
		nPhoneID = atoi(ppField[i]);
		nFrameNumber = atoi(ppField[i+1]);

		strcpy(strPhone,ptrArray_PhoneDict[nPhoneID]);
		charOne = strPhone[0];
		if('A' == charOne || 'E' == charOne || 'I' == charOne || 'O' == charOne || 'U' == charOne ||
			0 == strncmp(strPhone,"sil",3) || '#' == charOne);
		else
		{
			i += 3;
			nPhoneID_N = atoi(ppField[i]);
			nFrameNumber_N = atoi(ppField[i+1]);
			strcpy(strPhone_N,ptrArray_PhoneDict[nPhoneID_N]);

			//if(nFrameNumber+nFrameNumber_N < 12 )
			//{

			//}

			nFrameNumber += nFrameNumber_N;
			nShift = 3;
		}

		if(i+3 < nFieldNumber && nFrameNumber < 12 && i > 2)
		{


			nPhoneID_P = atoi(ppField[i-3-nShift]);
			strcpy(strPhone_P,ptrArray_PhoneDict[nPhoneID_P]);

			if(0 == strncmp(strPhone_P,"sil",3))
			{
				nPhoneID_N_N = atoi(ppField[i+3]);
				strcpy(strPhone_N_N,ptrArray_PhoneDict[nPhoneID_N_N]);
				if(0 == strncmp(strPhone_N_N,"sil",3))
				{
					charFlag = 'Y';
					nFrameNumber = nFrameNumber;				
				}
			}
		}


		i += 3;
	}

	free(pBuffer_PhoneDict);
	free(ptrArray_PhoneDict);

	free(pBuffer_WordDict);
	free(ptrArray_WordDict);

	free(pBuffer_AlignPhone);
	free(ptrArray_AlignPhone);

	free(pBuffer_AlignWord);
	free(ptrArray_AlignWord);


	return 0;
}

int YT_ASR_CheckResults_V2(char *strDictFile_Phone, 
			       char *strDictFile_Word,
			       char *strWaveFile_IN,
			       char *strWaveFile_OUT,														
			       char *strAlignPhone_IN,
			       char *strAlignPhone_OUT)							
{
	char **ptrArray_PhoneDict, *pBuffer_PhoneDict;
	unsigned int nEntryNumber_PhoneDict;


	char **ptrArray_WordDict, *pBuffer_WordDict;
	unsigned int nEntryNumber_WordDict;


	char strEntry[3000];
	char *ppField[1000];
	unsigned int nFieldNumber;

	unsigned int nStartFrameIndex = 0, nEndFrameIndex = 0;
	unsigned int i,j,nShift;

	char strPhone_P[100],strPhone[100],strPhone_N[100],strPhone_N_N[100];
	char strSyllable[200];
	unsigned int nPhoneID_P, nPhoneID, nPhoneID_N,nPhoneID_N_N;
	unsigned int nFrameNumber, nFrameNumber_N;
	unsigned char charOne, charFlag;

	unsigned int nResLen = 0;

	TEMP_AmplifyWaveformOnly(strWaveFile_IN,strWaveFile_OUT);


	ptrArray_PhoneDict = YT_LoadOneTextFileWithKey(0,strDictFile_Phone,&pBuffer_PhoneDict, &nEntryNumber_PhoneDict);
	assert(NULL != ptrArray_PhoneDict);


	ptrArray_WordDict = YT_LoadOneTextFileWithKey(0,strDictFile_Word,&pBuffer_WordDict, &nEntryNumber_WordDict);
	assert(NULL != ptrArray_WordDict);

	charFlag = 'N';	
	strcpy(strEntry,strAlignPhone_IN);


	i = 0;
	while(1)
	{
		charOne = strEntry[i];
		if('\0' == charOne) break;
		if('(' == charOne)
		{
			i ++;
			break;
		}

		i ++;
	}

	j = 0;
	while(1)
	{
		charOne = strEntry[i];
		if('\0' == charOne) break;
		if(charOne <= '9' && charOne >= '0')strEntry[j++] = charOne;
		else strEntry[j++] = 0x20;
		i ++;
	}
	strEntry[j] = '\0';



	YT_GetFields_NEW(1000,strEntry,ppField,&nFieldNumber);
	strSyllable[0] = '\0';
	strAlignPhone_OUT[0] = '\0';
	nResLen = 0;
	i = 0;
	while(i < nFieldNumber)
	{
		nShift = 0;
		nPhoneID = atoi(ppField[i]);
		nFrameNumber = atoi(ppField[i+1]);

		strcpy(strPhone,ptrArray_PhoneDict[nPhoneID]);
		j = 0;
		while(1)
		{
			charOne = strPhone[j];
			if('\0' == charOne) break;

			if(0x20 == charOne)
			{
				strPhone[j] = '\0';
				break;
			}

			if('_' == charOne)
			{

				strPhone[j] = '\0';
				break;
			}

			strAlignPhone_OUT[nResLen++] = charOne;
			j ++;
		}	
			
			
		charOne = strPhone[0];
		if('A' == charOne || 'E' == charOne || 'I' == charOne || 'O' == charOne || 'U' == charOne ||
			0 == strncmp(strPhone,"sil",3) || '#' == charOne)
		{



			strAlignPhone_OUT[nResLen++]  = 0x20;
			strAlignPhone_OUT[nResLen]  = '\0';
			//strcpy(&strAlignPhone_OUT[nResLen],strPhone);
			

		}
		else
		{
			i += 2;
			nPhoneID_N = atoi(ppField[i]);
			nFrameNumber_N = atoi(ppField[i+1]);

			strAlignPhone_OUT[nResLen++]  = '_';
			strcpy(strPhone_N,ptrArray_PhoneDict[nPhoneID_N]);

			j = 0;
			while(1)
			{
				charOne = strPhone_N[j];
				if('\0' == charOne) break;
				if(0x20 == charOne)
				{
					strPhone_N[j] = '\0';
					break;
				}

				if('_' == charOne)
				{
					strPhone_N[j] = '\0';
					break;
				}

				strAlignPhone_OUT[nResLen++] = charOne;

				j ++;
			}

			strAlignPhone_OUT[nResLen++] = 0x20;
			strAlignPhone_OUT[nResLen] = '\0';
			sprintf(strSyllable,"%s_%s",strPhone,strPhone_N);



			nFrameNumber += nFrameNumber_N;

			if(nFrameNumber > 40)
			{
				j = 0;
				while(1)
				{
					charOne = strSyllable[j];
					if('\0' == charOne) break;
					strAlignPhone_OUT[nResLen++] = charOne;
					j++;
				}

				strAlignPhone_OUT[nResLen++] = 0x20;
				strAlignPhone_OUT[nResLen] = '\0';

			}

			nShift = 2;
		}


		if(i+2 < nFieldNumber && nFrameNumber < 12 && i > 2)
		{
			nPhoneID_P = atoi(ppField[i-2-nShift]);
			strcpy(strPhone_P,ptrArray_PhoneDict[nPhoneID_P]);

			if(0 == strncmp(strPhone_P,"sil",3))
			{
				nPhoneID_N_N = atoi(ppField[i+3]);
				strcpy(strPhone_N_N,ptrArray_PhoneDict[nPhoneID_N_N]);
				if(0 == strncmp(strPhone_N_N,"sil",3))
				{
					charFlag = 'Y';
					nFrameNumber = nFrameNumber;				
				}
			}
		}



		i += 2;
	}


	strAlignPhone_OUT[nResLen] = '\0';

	free(pBuffer_PhoneDict);
	free(ptrArray_PhoneDict);

	free(pBuffer_WordDict);
	free(ptrArray_WordDict);
	return 0;
}						  

int YT_ASR_CheckResults_V3(char *strRecResult_Phone_IN,
						   char *strRecResult_Word_IN,

						   char **ptrArray_TTS_ASR_TAB, 
						   unsigned int nEntryNumber_TTS_ASR_TAB,

						   char **ptrArray_SWAP,
						   unsigned int nEntryNumber_SWAP,
						  
						   char **ptrArray_PhoneDict,
						   unsigned int nEntryNumber_PhoneDict,
						  
						   /*char *strWaveFile_NEW,*/

						   char **ptrArray_WordDict,
						   unsigned int nEntryNumber_WordDict,

						   char *strRecResult_Phone_OUT,
						   char *strRecResult_Word_OUT)							
{
	char strEntry[1000];
	char *ppField[300];
	unsigned int nFieldNumber;

	unsigned int nStartFrameIndex = 0, nEndFrameIndex = 0;
	unsigned int i,j,nShift;

	char strPhone_P[100]="",strPhone[100],strPhone_N[100],strPhone_N_N[100] = "";

	char strSyllable[200], strLastSyllable[200];
	unsigned int nPhoneID_P = 0, nPhoneID, nPhoneID_N,nPhoneID_N_N = 0;

	unsigned int nFrameNumber, nFrameNumber_N, nFrameNumber_N_LAST;
	unsigned char charOne, charFlag, charFlag_AllDigit, charDigit;

	unsigned int nResLen = 0, nLen;

	unsigned int nPhoneNumber, nPhoneNumberFromWord = 0;
	unsigned int nDigitNumber = 0;

	int nIndex, nClose;


	char *strSet_DigitFlag[] = 
	{
		"B A1 8",
		//"B AI3 100",
		"DU OG4 0",
		"G OU1 9",
		"GS ER4 2",
		"GU AI3 7",

		"JI OU3 9",
		"LI AG3 2",
		"LI IG2 0",
		"LI OU4 6",

		"QI I1 7",
		"S AN1 3",
		"S IH4 4",
		//"SH IH2 10",

		"W U3 5",
		"Y AO1 1",
		"Y I1 1"
	};

	unsigned int nNumber_DigitFlag = sizeof(strSet_DigitFlag) / sizeof(char*);

	charFlag = 'N';	
	strcpy(strEntry,strRecResult_Phone_IN);
	if('<' == strEntry[0])
	{
		i = 0;
		while(1)
		{
			charOne = strEntry[i];
			strEntry[i] = 0x20;
			if('>' == charOne || 0x20 == charOne) break;
			if('\0' == charOne) break;
			i ++;			
		}
	}

	i = 0;
	while(1)
	{
		charOne = strEntry[i];
		if('\0' == charOne) break;
		if('(' == charOne)
		{
			i ++;
			break;
		}

		i ++;
	}

	j = 0;
	while(1)
	{
		charOne = strEntry[i];
		if('\0' == charOne) break;
		if(charOne <= '9' && charOne >= '0')strEntry[j++] = charOne;
		else strEntry[j++] = 0x20;
		i ++;
	}
	strEntry[j] = '\0';



	YT_GetFields_NEW(300,strEntry,ppField,&nFieldNumber);
	nPhoneNumber = nFieldNumber / 2;

	nFrameNumber_N_LAST = 0;
	strLastSyllable[0] = '\0';

	//exclude the heading and trailing silence
	charFlag_AllDigit = 'Y';
	i = 0;
	while(i < nFieldNumber)
	{
		nShift = 0;
		nPhoneID = atoi(ppField[i]);
		nFrameNumber = atoi(ppField[i+1]);

		//AN1_I 72
		strcpy(strPhone,ptrArray_PhoneDict[nPhoneID]);
		j = 0;
		while(1)
		{
			charOne = strPhone[j];
			if('\0' == charOne) break;

			if(0x20 == charOne)
			{
				strPhone[j] = '\0';
				break;
			}

			if('_' == charOne)
			{
				strPhone[j] = '\0';
				break;
			}

			j ++;
		}

		charOne = strPhone[0];
		if(0 == strncmp(strPhone,"sil",3) || '#' == charOne)
		{
			nShift = 0;			
		}
		else
		{
			i += 2;
			
			/* add by Zhiming Wang */
			if(i >= nFieldNumber)  break ;

			nPhoneID_N = atoi(ppField[i]);

			nFrameNumber_N_LAST = atoi(ppField[i+1]);

			strcpy(strPhone_N,ptrArray_PhoneDict[nPhoneID_N]); 	//AN1_I 72
			j = 0;
			while(1)
			{
				charOne = strPhone_N[j];
				if('\0' == charOne) break;

				if(0x20 == charOne)
				{
					strPhone_N[j] = '\0';
					break;
				}

				if('_' == charOne)
				{
					strPhone_N[j] = '\0';
					break;
				}

				j ++;
			}	

			sprintf(strSyllable,"%s %s",strPhone,strPhone_N);
			nLen = strlen(strSyllable);
			nIndex = YT_StringBinarySearchWithLength(strSyllable,(char **)strSet_DigitFlag,0,nNumber_DigitFlag,&nClose,nLen);
			
			strcpy(strLastSyllable,strSyllable);
			if(-1 == nIndex)
			{
				charFlag_AllDigit = 'N';
			}

		}

		i += 2;
	}
	
	if('Y' == charFlag_AllDigit && nPhoneNumber > 5)
	{
		//convert syllable into digit

		i = 0;
		while(i < nFieldNumber)
		{

			nShift = 0;
			nPhoneID = atoi(ppField[i]);
			nFrameNumber = atoi(ppField[i+1]);

			strcpy(strPhone,ptrArray_PhoneDict[nPhoneID]); //AN1_I 72

			j = 0;
			while(1)
			{
				charOne = strPhone[j];
				if('\0' == charOne) break;

				if(0x20 == charOne)
				{
					strPhone[j] = '\0';
					break;
				}

				if('_' == charOne)
				{
					strPhone[j] = '\0';
					break;
				}

				j ++;
			}	
			

			charOne = strPhone[0];
			if(0 == strncmp(strPhone,"sil",3) || '#' == charOne)
			{
				nShift = 0;
			}
			else
			{
				i += 2;

				nPhoneID_N = atoi(ppField[i]);
				nFrameNumber_N = atoi(ppField[i+1]);
				strcpy(strPhone_N,ptrArray_PhoneDict[nPhoneID_N]);//AN1_I 72
				j = 0;
				while(1)
				{
					charOne = strPhone_N[j];
					if('\0' == charOne) break;

					if(0x20 == charOne)
					{
						strPhone_N[j] = '\0';
						break;
					}

					if('_' == charOne)
					{
						strPhone_N[j] = '\0';
						break;
					}

					j ++;
				}	



				sprintf(strSyllable,"%s %s",strPhone,strPhone_N);

				nLen = strlen(strSyllable);
				nIndex = YT_StringBinarySearchWithLength(strSyllable,(char **)strSet_DigitFlag,0,nNumber_DigitFlag,&nClose,nLen);

				//get the digit...
				j = nLen + 1;
				while(1)
				{
					charDigit = strSet_DigitFlag[nIndex][j];
					if('\0' == charDigit) break;
					if(charDigit <= '9' && charDigit >= '0') break;
					j ++;
				}

				nFrameNumber += nFrameNumber_N;

				if(nFrameNumber > 8)strRecResult_Word_OUT[nResLen++] = charDigit;

				if(nFrameNumber > 48 )
				{
					if(0 == strcmp(strSyllable,"W U3"))
					{
						//we need to add a '5'
						strRecResult_Word_OUT[nResLen++] = charDigit;
					}

				}


			}



			i += 2;
		}


		strRecResult_Word_OUT[nResLen] = '\0';
	}

	//52 hao
	if('N' == charFlag_AllDigit && nFrameNumber_N_LAST < 5 && 0 == strcmp(strLastSyllable,"H AO4"))
	{

		//remove "hao"
		nResLen = strlen(strRecResult_Word_OUT);
		j = 0;
		while(1)
		{
			if(0 == nResLen) break;
			charOne = strRecResult_Word_OUT[nResLen - 1];
			if(charOne < 0x21)
			{
				if(j >= 3) break;				
			}
			if(charOne > 0x80)
			{
				strRecResult_Word_OUT[nResLen - 1] = '\0';
				j ++;
				if(j >= 3) break;
			}

			nResLen --;
		}


		
	}



	return 0;
}


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
int YT_ASR_CheckRecResult_NEW(/*char *strWaveFile_IN,*/
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

							  /*char *strWaveFile_OUT,*/
							  char *strRecResult_Phone_OUT,
							  char *strRecResult_Word_OUT)
{
	char strEntry[2000];
	char *ppField[600];
	unsigned int nFieldNumber;

	unsigned int i, j,k, nSyllableNumber, nStringLen;
	unsigned char charOne, charNext,charTemp, charTone, charFlag;

	int nIndex, nClose;
	char strPhoneSequence[600], strKeyWord[200];
	unsigned int nPhoneSequenceLen, nTemp, nResLen;


	//the default output phone sequence for recognition result	
	strcpy(strRecResult_Phone_OUT,strRecResult_Phone_IN);
	if('<' == strRecResult_Phone_OUT[0])
	{
		i = 0;
		while(1)
		{
			charOne = strRecResult_Phone_OUT[i];
			strRecResult_Phone_OUT[i] = 0x20;
			if('>' == charOne || 0x20 == charOne) break;
			if('\0' == charOne) break;
			i ++;			
		}
	}

	//STEP 1: amplify waveform to ease further processing
	//TEMP_AmplifyWaveformOnly(strWaveFile_IN,strWaveFile_OUT);


	//STEP 2: check word sequence directly
	//if there are consequential syllables and they can compose a Chinese keyword, we transform them into a single keyword.
	nResLen = 0;
	strcpy(strEntry,strRecResult_Word_IN);
	if('<' == strEntry[0])
	{
		i = 0;
		while(1)
		{
			charOne = strEntry[i];
			strEntry[i] = 0x20;
			if('>' == charOne || 0x20 == charOne) break;
			if('\0' == charOne) break;
			i ++;			
		}
	}



	YT_GetFields_NEW(600,strEntry,ppField,&nFieldNumber);
	i = 0;
	while(i < nFieldNumber)
	{
		charOne = ppField[i][0];
		if(charOne <= 'z' && charOne >= 'a')
		{
			//get the following syllables...
			nSyllableNumber = 1;
			j = i+ 1;

			while(j < nFieldNumber)
			{
				charOne = ppField[j][0];
				if(charOne <= 'z' && charOne >= 'a') 
				{
					nSyllableNumber ++;
					j ++;
				} else break;
			}


			if(nSyllableNumber > 1)
			{
				//we need to see if they can form a keyword
				//convert each syllable into ASR phone sequence
				charFlag = 'Y';
				nPhoneSequenceLen = 0;
				strKeyWord[0] = '\0';
				for(k=i; k<j; k ++)
				{
					nStringLen = strlen(ppField[k]);
					charTone = ppField[k][nStringLen-1];


//					assert(charTone <= '5' && charTone >= '1');
					assert(charTone <= '5' && charTone >= '0');

//					if(charTone > '5' || charTone < '1')  charFlag = 'N';
					if(charTone > '5' || charTone < '0')  charFlag = 'N';

					if('5' == charTone) charTone = '0';

//					ppField[k][nStringLen-1] = '\0';
					ppField[k][nStringLen] = '\0';

					nIndex = YT_StringBinarySearchWithLength(ppField[k],ptrArray_TTS_ASR_TAB,0,nEntryNumber_TTS_ASR_TAB,&nClose,nStringLen-1);
					if(-1 != nIndex)
					{
						nTemp = nStringLen;
						while(1)
						{
							charTemp = ptrArray_TTS_ASR_TAB[nIndex][nTemp];
							if('\0' == charTemp) break;
							if(charTemp > 0x1f)
							{
								strPhoneSequence[nPhoneSequenceLen++] = charTemp;
							}
							nTemp ++;
						}
						
						strPhoneSequence[nPhoneSequenceLen++] = charTone;//add the tone
						strPhoneSequence[nPhoneSequenceLen++] = 0x20; //separate flag
						strPhoneSequence[nPhoneSequenceLen] = '\0';
					}
					else
					{
						charFlag = 'N';
					}

					if('N' == charFlag) break;
				}


				if('Y' == charFlag)
				{
					//conduct search using the phone sequence to see if they can compose a keyword
					strPhoneSequence[nPhoneSequenceLen] = '\0';


					//in case it requires normalization
					nTemp = 0;
					k = 0;
					while(1)
					{
						charOne = strPhoneSequence[k];
						if('\0' == charOne) break;
						if(charOne > 0x20)strPhoneSequence[nTemp ++] = charOne;
						else
						{
							if(0x20 == charOne)
							{
								charNext = strPhoneSequence[k+1];
								if(charNext > 0x20)strPhoneSequence[nTemp ++] = charOne;
							}							
						}

						k ++;
					}


					strPhoneSequence[nTemp ++] = 0x20;
					strPhoneSequence[nTemp ++] = '$';
					strPhoneSequence[nTemp ++] = '$';
					strPhoneSequence[nTemp ++] = '$';
					strPhoneSequence[nTemp ] = '\0';
					nPhoneSequenceLen = nTemp;

					nIndex = YT_StringBinarySearchWithLength(strPhoneSequence,ptrArray_SWAP,0,nEntryNumber_SWAP,&nClose,nPhoneSequenceLen);
					if(-1 != nIndex)
					{
						//get the keyword...
						k = nPhoneSequenceLen + 1;
						nTemp = 0;
						while(1)
						{
							charTemp = ptrArray_SWAP[nIndex][k];
							if('\0' == charTemp) break;

							strKeyWord[nTemp++] = charTemp;
							k ++;
						}

						strKeyWord[nTemp] = '\0';
						
					}
					else  charFlag = 'N'; //set the flag to false 

				}


				if('Y' == charFlag)
				{
					//concatenate the keyword...
					k = 0;
					while(1)
					{
						charTemp = strKeyWord[k];
						if('\0' == charTemp) break;
						strRecResult_Word_OUT[nResLen ++] = charTemp;
						k ++;
					}
					strRecResult_Word_OUT[nResLen ++] = 0x20;
					strRecResult_Word_OUT[nResLen] = '\0';

				}
				else
				{
					//concatenate the current field...
					k = 0;
					while(1)
					{
						charTemp = ppField[i][k];
						if('\0' == charTemp) break;
						strRecResult_Word_OUT[nResLen ++] = charTemp;
						k ++;
					}
					strRecResult_Word_OUT[nResLen ++] = 0x20;
					strRecResult_Word_OUT[nResLen] = '\0';

					j = i+1; // we don't match the keyword, so just move one field 
				}
			}  // end if(nSyllableNumber > 1)
			else	// [added by FHL] Now , nSyllableNumber == 1
			{
				k = 0;
				while(1)
				{
					charTemp = ppField[i][k];
					if('\0' == charTemp) break;
					strRecResult_Word_OUT[nResLen ++] = charTemp;
					k ++;
				}
				strRecResult_Word_OUT[nResLen ++] = 0x20;
				strRecResult_Word_OUT[nResLen] = '\0';
			}

			i = j; //move forward...
		}
		else
		{
			//concatenate the current field
			k = 0;
			while(1)
			{
				charTemp = ppField[i][k];
				if('\0' == charTemp) break;
				strRecResult_Word_OUT[nResLen ++] = charTemp;
				k ++;
			}
			strRecResult_Word_OUT[nResLen ++] = 0x20;
			strRecResult_Word_OUT[nResLen] = '\0';

			i ++;
		}
	}


	//STEP 3: check phone sequence
	//we hope to check word sequence by checking phone sequence
	//only for the all-digit string case now
	YT_ASR_CheckResults_V3(strRecResult_Phone_IN,
						   strRecResult_Word_IN,
						   ptrArray_TTS_ASR_TAB, nEntryNumber_TTS_ASR_TAB,						   
						   ptrArray_SWAP,nEntryNumber_SWAP,						  
						   ptrArray_PhoneDict,nEntryNumber_PhoneDict,						 
						   /*strWaveFile_OUT,*/
						   ptrArray_WordDict,nEntryNumber_WordDict,
						   strRecResult_Phone_OUT,
						   strRecResult_Word_OUT);


	//we also can remove too short digit!!!


	return 0;
}



// added by FHL
int YT_ASR_CheckRecResult_NEW_V2(/*char *strWaveFile_IN,*/
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

							  /*char *strWaveFile_OUT,*/
							  char *strRecResult_Phone_OUT,
							  char *strRecResult_Word_OUT)
{
	char strEntry[2000];
	char *ppField[600];
	unsigned int nFieldNumber;

	unsigned int i, j,k, nSyllableNumber, nStringLen;
	unsigned char charOne, charNext,charTemp, charTone, charFlag;

	int nIndex, nClose;
	char strPhoneSequence[600], strKeyWord[200];
	unsigned int nPhoneSequenceLen, nTemp, nResLen;


	//the default output phone sequence for recognition result
	strcpy(strRecResult_Phone_OUT,strRecResult_Phone_IN);
	if('<' == strRecResult_Phone_OUT[0])
	{
		i = 0;
		while(1)
		{
			charOne = strRecResult_Phone_OUT[i];
			strRecResult_Phone_OUT[i] = 0x20;
			if('>' == charOne || 0x20 == charOne) break;
			if('\0' == charOne) break;
			i ++;
		}
	}


	//STEP 1: amplify waveform to ease further processing
	//TEMP_AmplifyWaveformOnly(strWaveFile_IN,strWaveFile_OUT);


	//STEP 2: check word sequence directly
	//if there are consequential syllables and they can compose a Chinese keyword, we transform them into a single keyword.
	nResLen = 0;
	strcpy(strEntry,strRecResult_Word_IN);
	if('<' == strEntry[0])
	{
		i = 0;
		while(1)
		{
			charOne = strEntry[i];
			strEntry[i] = 0x20;
			if('>' == charOne || 0x20 == charOne) break;
			if('\0' == charOne) break;
			i ++;
		}
	}



	YT_GetFields_NEW(600,strEntry,ppField,&nFieldNumber);
	i = 0;
	while(i < nFieldNumber)
	{
		charOne = ppField[i][0];
		if(charOne <= 'z' && charOne >= 'a')
		{
			//get the following syllables...
			nSyllableNumber = 1;
			j = i+ 1;

			while(j < nFieldNumber)
			{
				charOne = ppField[j][0];
				if(charOne <= 'z' && charOne >= 'a')
				{
					nSyllableNumber ++;
					j ++;
				} else break;
			}


			if(nSyllableNumber > 1)
			{
				//we need to see if they can form a keyword
				//convert each syllable into ASR phone sequence
				charFlag = 'Y';
				nPhoneSequenceLen = 0;
				strKeyWord[0] = '\0';
				for(k=i; k<j; k ++)
				{
					nStringLen = strlen(ppField[k]);
					charTone = ppField[k][nStringLen-1];


//					assert(charTone <= '5' && charTone >= '1');
					assert(charTone <= '5' && charTone >= '0');

//					if(charTone > '5' || charTone < '1')  charFlag = 'N';
					if(charTone > '5' || charTone < '0')  charFlag = 'N';

					if('5' == charTone) charTone = '0';

//					ppField[k][nStringLen-1] = '\0';
					ppField[k][nStringLen] = '\0';

					nIndex = YT_StringBinarySearchWithLength(ppField[k],ptrArray_TTS_ASR_TAB,0,nEntryNumber_TTS_ASR_TAB,&nClose,nStringLen-1);
					if(-1 != nIndex)
					{
						nTemp = nStringLen;
						while(1)
						{
							charTemp = ptrArray_TTS_ASR_TAB[nIndex][nTemp];
							if('\0' == charTemp) break;
							if(charTemp > 0x1f)
							{
								strPhoneSequence[nPhoneSequenceLen++] = charTemp;
							}
							nTemp ++;
						}

//						strPhoneSequence[nPhoneSequenceLen++] = charTone;//add the tone		// FHL cancel this line
						strPhoneSequence[nPhoneSequenceLen++] = 0x20; //separate flag
						strPhoneSequence[nPhoneSequenceLen] = '\0';
					}
					else
					{
						charFlag = 'N';
					}

					if('N' == charFlag) break;
				}

				if('Y' == charFlag)
				{
					//conduct search using the phone sequence to see if they can compose a keyword
					strPhoneSequence[nPhoneSequenceLen] = '\0';


					//in case it requires normalization
					nTemp = 0;
					k = 0;
					while(1)
					{
						charOne = strPhoneSequence[k];
						if('\0' == charOne) break;
						if(charOne > 0x20)strPhoneSequence[nTemp ++] = charOne;
						else
						{
							if(0x20 == charOne)
							{
								charNext = strPhoneSequence[k+1];
								if(charNext > 0x20)strPhoneSequence[nTemp ++] = charOne;
							}
						}

						k ++;
					}


					strPhoneSequence[nTemp ++] = 0x20;
					strPhoneSequence[nTemp ++] = '$';
					strPhoneSequence[nTemp ++] = '$';
					strPhoneSequence[nTemp ++] = '$';
					strPhoneSequence[nTemp ] = '\0';
					nPhoneSequenceLen = nTemp;

					nIndex = YT_StringBinarySearchWithLength(strPhoneSequence,ptrArray_SWAP,0,nEntryNumber_SWAP,&nClose,nPhoneSequenceLen);
					if(-1 != nIndex)
					{
						//get the keyword...
						k = nPhoneSequenceLen + 1;
						nTemp = 0;
						while(1)
						{
							charTemp = ptrArray_SWAP[nIndex][k];
							if('\0' == charTemp) break;

							strKeyWord[nTemp++] = charTemp;
							k ++;
						}

						strKeyWord[nTemp] = '\0';
					}
					else  charFlag = 'N'; //set the flag to false

				}


				if('Y' == charFlag)
				{
					//concatenate the keyword...
					k = 0;
					while(1)
					{
						charTemp = strKeyWord[k];
						if('\0' == charTemp) break;
						strRecResult_Word_OUT[nResLen ++] = charTemp;
						k ++;
					}
					strRecResult_Word_OUT[nResLen ++] = 0x20;
					strRecResult_Word_OUT[nResLen] = '\0';

				}
				else
				{
					//concatenate the current field...
					k = 0;
					while(1)
					{
						charTemp = ppField[i][k];
						if('\0' == charTemp) break;
						strRecResult_Word_OUT[nResLen ++] = charTemp;
						k ++;
					}
					strRecResult_Word_OUT[nResLen ++] = 0x20;
					strRecResult_Word_OUT[nResLen] = '\0';

					j = i+1; // we don't match the keyword, so just move one field
				}
			}  // end if(nSyllableNumber > 1)
			else	// [added by FHL] Now , nSyllableNumber == 1
			{
				k = 0;
				while(1)
				{
					charTemp = ppField[i][k];
					if('\0' == charTemp) break;
					strRecResult_Word_OUT[nResLen ++] = charTemp;
					k ++;
				}
				strRecResult_Word_OUT[nResLen ++] = 0x20;
				strRecResult_Word_OUT[nResLen] = '\0';
			}

			i = j; //move forward...
		}
		else
		{
			//concatenate the current field
			k = 0;
			while(1)
			{
				charTemp = ppField[i][k];
				if('\0' == charTemp) break;
				strRecResult_Word_OUT[nResLen ++] = charTemp;
				k ++;
			}
			strRecResult_Word_OUT[nResLen ++] = 0x20;
			strRecResult_Word_OUT[nResLen] = '\0';

			i ++;
		}
	}


	//STEP 3: check phone sequence
	//we hope to check word sequence by checking phone sequence
	//only for the all-digit string case now
	YT_ASR_CheckResults_V3(strRecResult_Phone_IN,
						   strRecResult_Word_IN,
						   ptrArray_TTS_ASR_TAB, nEntryNumber_TTS_ASR_TAB,
						   ptrArray_SWAP,nEntryNumber_SWAP,
						   ptrArray_PhoneDict,nEntryNumber_PhoneDict,
						   /*strWaveFile_OUT,*/
						   ptrArray_WordDict,nEntryNumber_WordDict,
						   strRecResult_Phone_OUT,
						   strRecResult_Word_OUT);

	//we also can remove too short digit!!!


	return 0;
}
