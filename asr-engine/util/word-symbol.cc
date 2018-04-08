/*
 * word-symbol.cc
 *
 *  Created on : 6/24/2014
 *      Author : zhiming.wang
 */

# include "word-symbol.h"

WordSymbol::WordSymbol() : word_symbol_table_(NULL)
{
	initialize() ;
}

WordSymbol::~WordSymbol()
{
	release() ;
}

/* initialize */
void WordSymbol::initialize()
{
	word_symbol_table_ = NULL ;
	size_ = 0 ;
}

/* release */
void WordSymbol::release()
{
	if(NULL != word_symbol_table_)
	{
		for(size_t i = 0 ; i < size_ ; ++i)
		{
			if(NULL != word_symbol_table_[i])
			{
				free(word_symbol_table_[i]) ;
				word_symbol_table_[i] = NULL ;
			}
		}
		free(word_symbol_table_) ;
		word_symbol_table_ = NULL ;
	}
	size_ = 0 ;
}

/* read word symbol file */
void WordSymbol::read(const char* word_symbol_file)
{
	FILE* f = fopen(word_symbol_file , "rb") ;
	if(NULL == f)
	{
		char err[100] ;
		sprintf(err , "error : failed to open word symbol file : %s" , word_symbol_file) ;
		error(err , __FILE__ , __LINE__) ;
	}

	char word[512] ;
	long int id ;
	size_ = 0 ;
	size_t len , index = 0 ;
	const size_t buffer_size_increase = 1000 ;
	size_t buffer_size = buffer_size_increase ;

	WordIDMap* word_id_ = (WordIDMap*)malloc(buffer_size * sizeof(WordIDMap)) ;
	if(NULL == word_id_)  error("no memory to be allocated" , __FILE__ , __LINE__) ;

	if(!feof(f))
	{
		fscanf(f , "%s" , word) ;
		fscanf(f , "%ld" , &id) ;
	}

	while(!feof(f))
	{
		len = strlen(word) ;
		word_id_[index].word_ = (char*)malloc((1 + len) * sizeof(char)) ;
		assert(NULL != word_id_[index].word_) ;
		memcpy((void*)word_id_[index].word_ , (const void*)word , len * sizeof(char)) ;
		word_id_[index].word_[len] = '\0' ;

		word_id_[index].id_ = id ;

		index++ ;
		size_ = (size_  > (size_t)(id + 1)) ? size_ : (size_t)(id + 1) ;

		if(index >= buffer_size)
		{
			buffer_size += buffer_size_increase ;
			word_id_ = (WordIDMap*)realloc(word_id_ , buffer_size * sizeof(WordIDMap)) ;
			if(NULL == word_id_)  error("no memory to be allocated" , __FILE__ , __LINE__) ;
		}

		fscanf(f , "%s" , word) ;
		fscanf(f , "%ld" , &id) ;
	}

	word_symbol_table_ =(char**)malloc(size_ * sizeof(char*)) ;
	if(NULL == word_symbol_table_)  error("no memory to be allocated" , __FILE__ , __LINE__) ;
	memset((void*)word_symbol_table_ , 0 , size_ * sizeof(char*)) ;

	for(size_t i = 0 ; i < index ; ++i)
	{
		id = word_id_[i].id_ ;
		len = strlen(word_id_[i].word_) ;
		word_symbol_table_[id] = (char*)malloc((1 + len) * sizeof(char)) ;
		assert(NULL != word_symbol_table_[id]) ;
		memcpy((void*)word_symbol_table_[id] , (const void*)word_id_[i].word_ , len * sizeof(char)) ;
		word_symbol_table_[id][len] = '\0' ;
	}

	for(size_t i = 0 ; i < size_ ; ++i)
	{
		free(word_id_[i].word_) ;
		word_id_[i].word_ = NULL ;
	}
	free(word_id_) ;
	word_id_ = NULL ;

	fclose(f) ;
}

const char* WordSymbol::find(size_t key) const
{
	if(key >= 0 && key < size_)
		return (const char*)(word_symbol_table_[key]) ;

	return (const char*)("") ;
}

size_t WordSymbol::word_symbol_table_size() const
{
	return size_ ;
}
