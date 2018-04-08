/*
 * cstring.h
 * Time:2014-11-18
 * Author:xutao
 */

#ifndef _CSTRING_H_
#define _CSTRING_H_
# include "../base/common.h"
#include <string.h>
class String{
public:
	
	 String() :  size_(0) ,  ptr(NULL) 
	{
		ptr = (char*)malloc(sizeof(char) ) ;

		ptr[0] = '\0' ;
	}

	~String(){

		release() ;
	}

	inline void release()
	{
		if(NULL != ptr){

		       free(ptr);

		       ptr=NULL;
		}

		size_  = 0 ;
	}
	
	inline bool empty() const
	{
	       	return 0 == size_ ;
	}

     	inline size_t size(){

             	return size_;
       	}

       	char* c_str() const{

	        	return ptr;
	}	

	void clear(){

		ptr = (char*)realloc(ptr , sizeof(char)) ;
		size_ = 0 ;
		ptr[0] = '\0' ;
  	}

	void operator +=(int32 num) ;

	void operator +=(const char* s) ;

	void operator +=(char s) ;

	void resize(size_t N) ;

	String& operator=( String& string) ;

	String& operator=(char* s) ;
	
private:
	size_t size_;

	char *ptr;
};


#endif //_CSTRING_H_