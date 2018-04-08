/*
 * cstring.h
 * Time:2014-11-18
 * Author:xutao
 */
# include "cstring.h"
# include <string.h> 
# include <stdlib.h>



void String::resize(size_t N){

	if(N > size_){

		ptr = (char*)realloc(ptr, sizeof(char) * (N + 1));
	}

}

void String::operator +=(int32 num){
	
	char str[33];
	
	snprintf(str, sizeof(str), "%d", num);

	size_t length =strlen(str);
	
	resize(size_ + length) ;
	
	for(size_t i = 0; i< length; i++)
		
		ptr[size_++] = str[i];
	
	ptr[size_] = '\0';
  }




void String::operator +=(const char*s)
  {	
	size_t length = strlen(s) ;

	resize(size_ + length) ;

	for(size_t i=0;i<length;i++){

	   ptr[size_++] = s[i] ;

	}

	ptr[size_] = '\0' ;
  }

 void String::operator +=( char s ){

 	resize(size_ + 1) ;
 	
 	ptr[size_++] = s ;

 	ptr[size_] = '\0' ;
 }

String& String::operator=( String& string)
  {
      if(ptr != string.c_str())
      {
	free(ptr) ;

	ptr = 0 ;

	int32 length = string.size() ;

	char* ptr_str = string.c_str() ;

             ptr = (char*)malloc(sizeof(char)*(length + 1)) ;
	
             strcpy(ptr, ptr_str); 

             size_ = length ;

      }

       return *this ;
  }
	
String& String::operator=(char* s) 
{
	this->clear() ;

	size_t length = strlen(s) ;

	ptr = (char*)realloc(ptr , sizeof(char) * (length+1)) ;

	size_ = 0 ;

	for(size_t i=0;i<length;i++){

	   ptr[size_++] = s[i] ;

	}

	ptr[size_] = '\0' ;
}