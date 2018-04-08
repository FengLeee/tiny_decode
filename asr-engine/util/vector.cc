/*
 * vector.cc
 *
 *  Created on : 08/06/2014
 *      Author : zhiming.wang
 */

# include "vector.h"

VectorF::VectorF(size_t N , real value) : data_(NULL) , size_(0) , capacity_(0)
{
	size_ = capacity_ = N ;
	Malloc(&data_ , N * sizeof(real)) ;
	for(size_t i = 0 ; i < N ; ++i)  data_[i] = value ;
}

VectorF::VectorF(const VectorF& other) : data_(NULL) , size_(0) , capacity_(0)
{
	capacity_ = other.capacity_ ;
	Malloc(&data_ , capacity_ * sizeof(real)) ;
	size_ = other.size_ ;
	memcpy((void*)data_ , (const void*)other.data_ , size_ * sizeof(real)) ;
}

VectorF& VectorF::operator=(const VectorF& other)
{
	if(0 != capacity_)  release() ;

	capacity_ = other.capacity_ ;
	Malloc(&data_ , capacity_ * sizeof(real)) ;
	size_ = other.size_ ;
	memcpy((void*)data_ , (const void*)other.data_ , size_ * sizeof(real)) ;

	return *this ;
}

VectorF::~VectorF()
{
	release() ;
}

void VectorF::resize(size_t N)
{
	if(0 != capacity_)
	{
		assert(NULL != data_) ;
		capacity_ = N << 1 ;
		real* data = (real*)realloc(data_ , capacity_ * sizeof(real)) ;
		if(NULL != data)  data_ = data ;
		else  error("error to re-allocate memory" , __FILE__ , __LINE__) ;
		memset((void*)(data_ + size_) , 0 , (capacity_ - size_) * sizeof(real)) ;
	} else {
		capacity_ = N << 1 ;
		Malloc(&data_ , capacity_ * sizeof(real)) ;
		memset((void*)data_ , 0 , capacity_ * sizeof(real)) ;
	}

	size_ = N ;
}

void VectorF::release()
{
	Free(&data_) ;
	size_ = capacity_ = 0 ;
}

void VectorF::push_back(real value)
{
	if(size_ == capacity_)
	{
		if(0 == capacity_)
		{
			capacity_ = 256 ;
			Malloc(&data_ , capacity_ * sizeof(real)) ;
		} else {
			capacity_ = capacity_ << 1 ;
			real* data = (real*)realloc(data_ , capacity_ * sizeof(real)) ;
			if(NULL != data)  data_ = data ;
			else  error("error to re-allocate memory" , __FILE__ , __LINE__) ;
		}
	}
	
	data_[size_++] = value ;
}

void VectorF::clear()
{
	release() ;
}

/**************************************************************************************************/

VectorI::VectorI(size_t N , real value) : data_(NULL) , size_(0) , capacity_(0)
{
	size_ = capacity_ = N ;
	Malloc(&data_ , N * sizeof(real)) ;
	for(size_t i = 0 ; i < N ; ++i)  data_[i] = value ;
}

VectorI::VectorI(const VectorI& other) : data_(NULL) , size_(0) , capacity_(0)
{
	capacity_ = other.capacity_ ;
	Malloc(&data_ , capacity_ * sizeof(real)) ;
	size_ = other.size_ ;
	memcpy((void*)data_ , (const void*)other.data_ , size_ * sizeof(real)) ;
}

VectorI& VectorI::operator=(const VectorI& other)
{
	if(0 != capacity_)  release() ;

	capacity_ = other.capacity_ ;
	Malloc(&data_ , capacity_ * sizeof(real)) ;
	size_ = other.size_ ;
	memcpy((void*)data_ , (const void*)other.data_ , size_ * sizeof(real)) ;

	return *this ;
}

VectorI::~VectorI()
{
	release() ;
}

void VectorI::resize(size_t N)
{
	if(0 != capacity_)
	{
		assert(NULL != data_) ;
		capacity_ = N << 1 ;
		real* data = (real*)realloc(data_ , capacity_ * sizeof(real)) ;
		if(NULL != data)  data_ = data ;
		else  error("error to re-allocate memory" , __FILE__ , __LINE__) ;
		memset((void*)(data_ + size_) , 0 , (capacity_ - size_) * sizeof(real)) ;
	} else {
		capacity_ = N << 1 ;
		Malloc(&data_ , capacity_ * sizeof(real)) ;
		memset((void*)data_ , 0 , capacity_ * sizeof(real)) ;
	}

	size_ = N ;
}

void VectorI::release()
{
	Free(&data_) ;
	size_ = capacity_ = 0 ;
}

void VectorI::push_back(real value)
{
	if(size_ == capacity_)
	{
		if(0 == capacity_)
		{
			capacity_ = 256 ;
			Malloc(&data_ , capacity_ * sizeof(real)) ;
		} else {
			capacity_ = capacity_ << 1 ;
			real* data = (real*)realloc(data_ , capacity_ * sizeof(real)) ;
			if(NULL != data)  data_ = data ;
			else  error("error to re-allocate memory" , __FILE__ , __LINE__) ;
		}
	}

	data_[size_++] = value ;
}

void VectorI::clear()
{
	release() ;
}

/**************************************************************************************************/

VectorArc::VectorArc(size_t N) : data_(NULL) , size_(0) , capacity_(0)
{
	size_ = capacity_ = N ;
	data_ = (real*)malloc(N * sizeof(real)) ;
	assert(NULL != data_) ;
}

VectorArc::VectorArc(size_t N , real value) : data_(NULL) , size_(0) , capacity_(0)
{
	size_ = capacity_ = N ;

	data_ = (real*)malloc(N * sizeof(real)) ;
	assert(NULL != data_) ;

	for(size_t i = 0 ; i < N ; ++i)  data_[i] = value ;
}

VectorArc::VectorArc(const VectorArc& other) : data_(NULL) , size_(0) , capacity_(0)
{
	capacity_ = other.capacity_ ;

	data_ = (real*)malloc(capacity_ * sizeof(real)) ;
	assert(NULL != data_) ;

	size_ = other.size_ ;
	memcpy((void*)data_ , (const void*)other.data_ , size_ * sizeof(real)) ;
}

VectorArc& VectorArc::operator=(const VectorArc& other)
{
	if(0 != capacity_)  release() ;

	capacity_ = other.capacity_ ;

	data_ = (real*)malloc(capacity_ * sizeof(real)) ;
	assert(NULL != data_) ;

	size_ = other.size_ ;
	memcpy((void*)data_ , (const void*)other.data_ , size_ * sizeof(real)) ;

	return *this ;
}

VectorArc::~VectorArc()
{
	release() ;
}

void VectorArc::resize(size_t N)
{
	if(0 != capacity_)
	{
		assert(NULL != data_) ;
		capacity_ = N << 1 ;
		real* data = (real*)realloc(data_ , capacity_ * sizeof(real)) ;
		if(NULL != data)  data_ = data ;
		else  error("error to re-allocate memory" , __FILE__ , __LINE__) ;
		memset((void*)(data_ + size_) , 0 , (capacity_ - size_) * sizeof(real)) ;
	} else {
		capacity_ = N << 1 ;

		data_ = (real*)malloc(capacity_ * sizeof(real)) ;
		assert(NULL != data_) ;

		memset((void*)data_ , 0 , capacity_ * sizeof(real)) ;
	}

	size_ = N ;
}

void VectorArc::release()
{
	if(NULL != data_)
	{
		for(size_t i = 0 ; i < size_ ; ++i)  data_[i] = NULL ;
		free(data_) ;
		data_ = NULL ;
	}

	size_ = capacity_ = 0 ;
}

void VectorArc::push_back(real value)
{
	if(size_ == capacity_)
	{
		if(0 == capacity_)
		{
			capacity_ = 256 ;
			data_ = (real*)malloc(capacity_ * sizeof(real)) ;
			assert(NULL != data_) ;
		} else {
			capacity_ = capacity_ << 1 ;
			real* data = (real*)realloc(data_ , capacity_ * sizeof(real)) ;
			if(NULL != data)  data_ = data ;
			else  error("error to re-allocate memory" , __FILE__ , __LINE__) ;
		}
	}

	data_[size_++] = value ;
}

void VectorArc::clear()
{
	release() ;
}
