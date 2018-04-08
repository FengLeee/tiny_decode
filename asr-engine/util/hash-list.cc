/*
 * hash-list.cc
 *
 *  Created on : 08/22/2014
 *      Author : zhiming.wang
 */

# include "hash-list.h"

HashList::~HashList()
{
	release() ;
}

/* initialize */
void HashList::initialize()
{
	list_head_ = NULL ;
	bucket_list_tail_ = (size_t)(-1) ;   /* invalid , but just used as a label */
	hash_size_ = 0 ;
	buckets_ = NULL ;
	buckets_size_ = 0 ;
	elem_length_ = 0 ;
	freed_head_ = NULL ;
	allocated_block_ = NULL ;
	allocated_block_number_ = 0 ;
}

/* release */
void HashList::release()
{
	list_head_ = NULL ;
	freed_head_ = NULL ;

	if(NULL != buckets_)
	{
		for(size_t i = 0 ; i < buckets_size_ ; ++i)  buckets_[i].initialize((size_t)(-1) , NULL) ;
		free(buckets_) ;  buckets_ = NULL ;  buckets_size_ = 0 ;
	}

	if(NULL != allocated_block_)
	{
		for(size_t i = 0 ; i < allocated_block_number_ ; ++i)
		{
			if(NULL != allocated_block_[i])
			{
				free(allocated_block_[i]) ;  allocated_block_[i] = NULL ;
			}
			
		}
		free(allocated_block_) ;  allocated_block_ = NULL ;  allocated_block_number_ = 0 ;
	}

	elem_length_ = 0 ;
}

void HashList::resize(size_t size)
{
	hash_size_ = size;
	assert(NULL == list_head_ && bucket_list_tail_ == (size_t)(-1)) ;  /*  make sure being empty  */

	if(size > buckets_size_)
	{
		HashBucket* buckets = (NULL == buckets_) ? (HashBucket*)malloc(sizeof(HashBucket) * size) : (HashBucket*)realloc(buckets_ , sizeof(HashBucket) * size) ;
		if(NULL != buckets)  buckets_ = buckets ;
		else  error("error to re-allocate memory" , __FILE__ , __LINE__) ;

		for(size_t i = buckets_size_ ; i != size ; ++i)  buckets_[i].initialize((size_t)(-1) , NULL) ;
		buckets_size_ = size ;
	}
}

HashList::Elem* HashList::clear()
{
	for(size_t cur_bucket = bucket_list_tail_ ; cur_bucket != (size_t)(-1) ; cur_bucket = buckets_[cur_bucket].prev_bucket_)
		buckets_[cur_bucket].last_elem_ = NULL ;  /* to indicate "empty" */

	bucket_list_tail_ = (size_t)(-1) ;
	Elem* cur_list = list_head_ ;
	list_head_ = NULL ;
	elem_length_ = 0 ;
	return cur_list ;
}

HashList::Elem* HashList::new_elem()
{
	if(freed_head_)
	{
		Elem* elem = freed_head_ ;
		freed_head_ = freed_head_->tail_ ;
		return elem ;
	} else {
		Elem* elem_array = (Elem*)malloc(sizeof(Elem) * allocate_block_size_) ;
		assert(NULL != elem_array) ;

		for(size_t i = 0 ; i < allocate_block_size_ - 1 ; ++i)
		{
			elem_array[i].key_ = 0 ;
			elem_array[i].value_ = NULL ;
			elem_array[i].tail_ = elem_array + i + 1 ;
		}
		elem_array[allocate_block_size_ - 1].initialize() ;

		freed_head_ = elem_array ;

		allocated_block_number_ += 1 ;

		Elem** allocated_block = (NULL == allocated_block_) ? (Elem**)malloc(sizeof(Elem*) * allocated_block_number_) : (Elem**)realloc(allocated_block_ , sizeof(Elem*) * allocated_block_number_) ;

		if(NULL != allocated_block)  allocated_block_ = allocated_block ;
		else  error("error to re-allocate memory" , __FILE__ , __LINE__) ;

		allocated_block_[allocated_block_number_ - 1] = elem_array ;

		return this->new_elem() ;
	}
}

void HashList::insert(int32 key , Token* token)
{
	size_t index = (((size_t)key) % hash_size_) ;

	HashBucket& bucket = buckets_[index] ;

	Elem* elem = new_elem() ;
	elem->key_ = key ;
	elem->value_ = token ;

	if(NULL == bucket.last_elem_)
	{
		if(bucket_list_tail_ == (size_t)(-1))
		{
			assert(NULL == list_head_) ;
			list_head_ = elem ;
		} else {
			buckets_[bucket_list_tail_].last_elem_->tail_ = elem ;
		}

		elem->tail_ = NULL ;
		bucket.last_elem_ = elem ;
		bucket.prev_bucket_ = bucket_list_tail_ ;
		bucket_list_tail_ = index ;
	} else {
		elem->tail_ = bucket.last_elem_->tail_ ;
		bucket.last_elem_->tail_ = elem ;
		bucket.last_elem_ = elem ;
	}

	elem_length_++ ;
}

HashList::Elem* HashList::find(int32 key)
{
	size_t index = (((size_t)key) % hash_size_) ;

	HashBucket& bucket = buckets_[index] ;

	if(NULL == bucket.last_elem_) {
		return NULL ;
	} else {
		Elem* head = (bucket.prev_bucket_ == (size_t)(-1)) ? list_head_ : buckets_[bucket.prev_bucket_].last_elem_->tail_ ;
		Elem* tail = bucket.last_elem_->tail_ ;

		for(Elem* elem = head ; elem != tail ; elem = elem->tail_)
			if(key == elem->key_)  return elem ;
	}

	return NULL ;
}
