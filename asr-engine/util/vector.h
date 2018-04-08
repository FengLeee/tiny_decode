/*
 * vector.h
 *
 *  Created on : 08/06/2014
 *      Author : zhiming.wang
 */

# ifndef _VECTOR_H_
# define _VECTOR_H_

# include "../base/common.h"
# include "wfst.h"

class Token
{
public :
	Arc_* arc_ ;
	Token* prev_ ;
	int32 ref_count_ ;
	BaseFloat weight_ ;  /*  weight up to current point  */

	inline Token() : arc_(NULL) , prev_(NULL) , ref_count_(0) , weight_(INFINITY)
	{}

	inline ~Token()
	{
		Delete(this) ;
	}

	/* initialize token */
	inline void initialize(Arc_* arc , BaseFloat acoustic_weight , Token* prev)
	{
		assert(NULL != arc) ;

		arc_ = arc ;
		prev_ = prev ;
		ref_count_ = 1 ;

		if(prev) {
			prev->ref_count_ += 1 ;
			weight_ = Times(Times(prev->weight_ , arc->arc_weight) , acoustic_weight) ;
		} else {
			weight_ = Times(arc->arc_weight , acoustic_weight) ;
		}
	}

	/* initialize token */
	inline void initialize(Arc_* arc , Token* prev)
	{
		assert(NULL != arc) ;

		arc_ = arc ;
		prev_ = prev ;
		ref_count_ = 1 ;

		if(prev) {
			prev->ref_count_ += 1 ;
			weight_ = Times(prev->weight_ , arc->arc_weight) ;
		} else {
			weight_ = arc->arc_weight ;
		}
	}

	inline bool operator<(const Token& other)
	{
		/* this makes sense for log + tropical semi-ring */
		return weight_ > other.weight_ ;
	}

	inline static void Delete(Token* token)
	{
		if(NULL == token)  return ;
		
		while(--token->ref_count_ == 0)
		{
			Token *prev = token->prev_ ;
			token->arc_ = NULL ;
			free(token) ;  token = NULL ;
			if(prev == NULL)  return ;
			else token = prev ;
		}
		
		
	}

	inline static void Malloc(Token** token , Arc_* arc , BaseFloat acoustic_weight , Token* prev)
	{
		*token = (Token*)malloc(sizeof(Token)) ;
		if(NULL == *token)  error("memory allocation error , unable to allocate memory for token" , __FILE__ , __LINE__) ;

		(*token)->initialize(arc , acoustic_weight , prev) ;
	}

	inline static void Malloc(Token** token , Arc_* arc , Token* prev)
	{
		*token = (Token*)malloc(sizeof(Token)) ;
		if(NULL == *token)  error("memory allocation error , unable to allocate memory for token" , __FILE__ , __LINE__) ;

		(*token)->initialize(arc , prev) ;
	}

	/*
	 * Tropical SemiRing , and multiply : plus
	*/
	inline static BaseFloat Times(BaseFloat a , BaseFloat b)
	{
		if(1 == ISINF(a))  return a ;
		else if(1 == ISINF(b))  return b ;
		else return a + b ;
	}
} ;

/**************************************************************************************************/

class VectorF
{
	typedef BaseFloat real ;
public :
	inline VectorF() : data_(NULL) , size_(0) , capacity_(0)  {}
	inline VectorF(size_t N) : data_(NULL) , size_(0) , capacity_(0)
	{
		size_ = capacity_ = N ;
		Malloc(&data_ , N * sizeof(real)) ;
	}
	VectorF(size_t N , real value) ;
	VectorF(const VectorF& other) ;
	VectorF& operator=(const VectorF& other) ;
	~VectorF() ;

	void resize(size_t N) ;

	void release() ;

	inline bool empty() const
	{
		return 0 == size_ ;
	}

	void push_back(real value) ;

	inline real& back()
	{
		assert(size_ >= 1) ;
		return data_[size_ - 1] ;
	}

	inline const real& back() const
	{
		assert(size_ >= 1) ;
		return data_[size_ - 1] ;
	}

	inline void pop_back()
	{
		assert(size_ >= 1) ;
		size_ -= 1 ;
	}

	void clear() ;

	inline size_t size() const
	{
		return size_ ;
	}

	inline real* begin()
	{
		return &data_[0] ;
	}

	inline real* end()
	{
		return &data_[size_ - 1] ;
	}

	inline const real* begin() const
	{
		return (const real*)&data_[0] ;
	}

	inline const real* end() const
	{
		return (const real*)&data_[size_ - 1] ;
	}

	inline real& operator[](size_t index)
	{
		assert(index < size_) ;
		return data_[index] ;
	}

	inline const real& operator[](size_t index) const
	{
		assert(index < size_) ;
		return (const real&)data_[index] ;
	}

private :
	real* data_ ;
	size_t size_ ;
	size_t capacity_ ;

} ;

/**************************************************************************************************/

class VectorI
{
	typedef int32 real ;
public :
	inline VectorI() : data_(NULL) , size_(0) , capacity_(0)  {}
	inline VectorI(size_t N) : data_(NULL) , size_(0) , capacity_(0)
	{
		size_ = capacity_ = N ;
		Malloc(&data_ , N * sizeof(real)) ;
	}
	VectorI(size_t N , real value) ;
	VectorI(const VectorI& other) ;
	VectorI& operator=(const VectorI& other) ;
	~VectorI() ;

	void resize(size_t N) ;

	void release() ;

	inline bool empty() const
	{
		return 0 == size_ ;
	}

	void push_back(real value) ;

	inline real& back()
	{
		assert(size_ >= 1) ;
		return data_[size_ - 1] ;
	}

	inline const real& back() const
	{
		assert(size_ >= 1) ;
		return data_[size_ - 1] ;
	}

	inline void pop_back()
	{
		assert(size_ >= 1) ;
		size_ -= 1 ;
	}

	void clear() ;

	inline size_t size() const
	{
		return size_ ;
	}

	inline real* begin()
	{
		return &data_[0] ;
	}

	inline real* end()
	{
		return &data_[size_ - 1] ;
	}

	inline const real* begin() const
	{
		return (const real*)&data_[0] ;
	}

	inline const real* end() const
	{
		return (const real*)&data_[size_ - 1] ;
	}

	inline real& operator[](size_t index)
	{
		assert(index < size_) ;
		return data_[index] ;
	}

	inline const real& operator[](size_t index) const
	{
		assert(index < size_) ;
		return (const real&)data_[index] ;
	}

private :
	real* data_ ;
	size_t size_ ;
	size_t capacity_ ;

} ;

/**************************************************************************************************/

class VectorArc
{
	typedef Arc_* real ;
public :
	inline VectorArc() : data_(NULL) , size_(0) , capacity_(0)  {}
	VectorArc(size_t N) ;
	VectorArc(size_t N , real value) ;
	VectorArc(const VectorArc& other) ;
	VectorArc& operator=(const VectorArc& other) ;
	~VectorArc() ;

	void resize(size_t N) ;

	void release() ;

	inline bool empty() const
	{
		return 0 == size_ ;
	}

	void push_back(real value) ;

	inline real& back()
	{
		assert(size_ >= 1) ;
		return data_[size_ - 1] ;
	}

	inline const real& back() const
	{
		assert(size_ >= 1) ;
		return data_[size_ - 1] ;
	}

	inline void pop_back()
	{
		assert(size_ >= 1) ;
		size_ -= 1 ;
	}

	void clear() ;

	inline size_t size() const
	{
		return size_ ;
	}

	inline real* begin()
	{
		return &data_[0] ;
	}

	inline real* end()
	{
		return &data_[size_ - 1] ;
	}

	inline const real* begin() const
	{
		return (const real*)&data_[0] ;
	}

	inline const real* end() const
	{
		return (const real*)&data_[size_ - 1] ;
	}

	inline real& operator[](size_t index)
	{
		assert(index < size_) ;
		return data_[index] ;
	}

	inline const real& operator[](size_t index) const
	{
		assert(index < size_) ;
		return (const real&)data_[index] ;
	}

private :
	real* data_ ;
	size_t size_ ;
	size_t capacity_ ;

} ;

# endif /* _VECTOR_H_ */
