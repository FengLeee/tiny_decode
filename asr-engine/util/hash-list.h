/*
 * hash-list.h
 *
 *  Created on : 08/22/2014
 *      Author : zhiming.wang
 */

# ifndef _HASH_LIST_H_
# define _HASH_LIST_H_

# include "vector.h"

class HashList
{
public :
	struct Elem
	{
		int32 key_ ;
		Token* value_ ;
		Elem* tail_ ;

		inline Elem() : key_(0) , value_(NULL) , tail_(NULL)  {}

		inline void initialize()
		{
			key_ = 0 ;
			value_ = NULL ;
			tail_ = NULL ;
		}
	} ;

public :
	HashList() : list_head_(NULL) , bucket_list_tail_((size_t)-1) , hash_size_(0) , buckets_(NULL) , buckets_size_(0) , elem_length_(0) , freed_head_(NULL) , allocated_block_(NULL) , allocated_block_number_(0)  {}

	~HashList() ;

	/* initialize */
	void initialize() ;

	/* release */
	void release() ;

	void resize(size_t size) ;

	Elem* clear() ;

	inline Elem* get_list() const
	{
		return list_head_ ;
	}

	inline size_t size() const
	{
		return hash_size_ ;
	}

	inline void delete_elem(Elem* elem)
	{
		elem->tail_ = freed_head_ ;
		freed_head_ = elem ;
	}

	Elem* new_elem() ;

	void insert(int32 key , Token* token) ;

	Elem* find(int32 key) ;

	size_t elem_length()  const
	{
		return elem_length_ ;
	}

private :
	struct HashBucket
	{
		size_t prev_bucket_ ;    /*  index to next bucket(-1 if list tail)  */
	    Elem* last_elem_ ;       /*  pointer to last element in this bucket (NULL if empty)  */

	    inline HashBucket(size_t index , Elem* elem) : prev_bucket_(index) , last_elem_(elem)  {}

	    inline void initialize(size_t index , Elem* elem)
	    {
	    	prev_bucket_ = index ;
	    	last_elem_ = elem ;
	    }

	} ;

	Elem* list_head_ ;                                   /*  head of currently stored list  */

	size_t bucket_list_tail_ ;  		                 /*  tail of list of active hash buckets  */

	size_t hash_size_ ;  				                 /*  number of hash buckets  */

	HashBucket* buckets_ ;
	size_t buckets_size_ ;

	size_t elem_length_ ;

	Elem* freed_head_ ;  				                 /*  head of list of currently freed elements(ready for allocation)  */

	Elem** allocated_block_ ;  				             /*  list of allocated blocks  */
	size_t allocated_block_number_ ;

	static const size_t allocate_block_size_ = 1024 ;    /*  number of elements to allocate in one block  */

} ;

# endif /* _HASH_LIST_H_ */
