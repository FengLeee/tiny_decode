/*
 * unordered-set.h
 *
 *  Created on : 08/29/2014
 *      Author : zhiming.wang
 */

# ifndef _UNORDERED_SET_H_
# define _UNORDERED_SET_H_

# include "vector.h"

class UnorderedSet
{
public :
	struct Elem
	{
		Token* token_ ;
		Elem* tail_ ;

		inline Elem() : token_(NULL) , tail_(NULL)  {}

		inline void initialize()
		{
			token_ = NULL ;
			tail_ = NULL ;
		}

	} ;

public :
	UnorderedSet(size_t size = 0) ;
	~UnorderedSet() ;

	/* initialize */
	void initialize(size_t size = 0) ;

	/* release */
	void release() ;

	void resize(size_t size) ;

	Elem* clear() ;

	inline Elem* head() const
	{
		return head_ ;
	}

	inline void delete_elem(Elem* elem)
	{
		elem->tail_ = freed_head_ ;
		freed_head_ = elem ;
	}

	Elem* new_elem() ;

	void insert(Token* token) ;

	bool find(Token* token) ;

	size_t elem_length() const
	{
		return elem_length_ ;
	}

private :
	struct HashBucket
	{
		size_t prev_bucket_ ;      /*  index to next bucket(-1 if list tail)  */
		Elem* last_elem_ ;         /*  pointer to last element in this bucket (NULL if empty)  */

		inline HashBucket(size_t index , Elem* elem) : prev_bucket_(index) , last_elem_(elem)  {}

		inline void initialize(size_t index , Elem* elem)
		{
			prev_bucket_ = index ;
		    last_elem_ = elem ;
		}

	} ;

	inline size_t hash(Token* token)
	{
		size_t address = (size_t)(((uintptr_t)(const void *)token) & 0xFFFFFFFF) ;

		return (size_t)((address * 6643838879LL) >> 24) % hash_size_ ;
	}

	Elem* head_ ;                                         /*  head of currently stored list  */

	size_t bucket_tail_ ;  		                          /*  tail of list of active hash buckets  */

	size_t hash_size_ ;  				                  /*  number of hash buckets  */

	HashBucket* buckets_ ;
	size_t buckets_size_ ;

	size_t elem_length_ ;

	Elem* freed_head_ ;  				                 /*  head of list of currently freed elements(ready for allocation)  */

	Elem** allocated_block_ ;  				             /*  list of allocated blocks  */
	size_t allocated_block_number_ ;

	static const size_t allocate_block_size_ = 1024 ;    /*  number of elements to allocate in one block  */

} ;

# endif /* _UNORDERED_SET_H_ */
