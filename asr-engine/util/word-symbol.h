/*
 * word-symbol.h
 *
 *  Created on : 6/24/2014
 *      Author : zhiming.wang
 */

# ifndef _WORD_SYMBOL_H_
# define _WORD_SYMBOL_H_

# include "../base/common.h"

struct WordIDMap
{
	long int id_ ;
	char* word_ ;

	WordIDMap() : id_(0) , word_(NULL)  {}

} ;

class WordSymbol {
public :
	WordSymbol() ;
	~WordSymbol() ;

	/* initialize */
	void initialize() ;

	/* release */
	void release() ;

	/* read word symbol file */
	void read(const char* word_symbol_file) ;

	const char* find(size_t key) const ;

	size_t word_symbol_table_size() const ;

private :
	size_t size_ ;
	char** word_symbol_table_ ;

} ;

# endif /* _WORD_SYMBOL_H_ */
