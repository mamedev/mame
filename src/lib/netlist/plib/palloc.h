// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * palloc.h
 *
 */

#ifndef PALLOC_H_
#define PALLOC_H_

#include <exception>
#include <vector>
#include <memory>
#include <utility>

#include "pconfig.h"
#include "pstring.h"

//============================================================
//  exception base
//============================================================

class pexception : public std::exception
{
public:
	pexception(const pstring &text);
	virtual ~pexception() throw() {}

	const pstring &text() { return m_text; }

private:
	pstring m_text;
};

//============================================================
//  Memory allocation
//============================================================

#if (PSTANDALONE)
#include <cstddef>
#include <new>

#if defined(__GNUC__) && (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#if !defined(__ppc__) && !defined (__PPC__) && !defined(__ppc64__) && !defined(__PPC64__)
#define ATTR_ALIGN __attribute__ ((aligned(64)))
#else
#define ATTR_ALIGN
#endif
#else
#define ATTR_ALIGN
#endif

#define palloc(T)             new T
#define pfree(_ptr)           delete _ptr

#define palloc_array(T, N)    new T[N]
#define pfree_array(_ptr)     delete[] _ptr
#else

#define ATTR_ALIGN

template<typename T, typename... Args>
T *palloc(Args&&... args)
{
    return new T(std::forward<Args>(args)...);
}

template<typename T>
void pfree(T *ptr) { delete ptr; }

template<typename T>
inline T* palloc_array(std::size_t num)
{
	return new T[num]();
}

template<typename T>
void pfree_array(T *ptr) { delete [] ptr; }

#endif

template<typename T, typename... Args>
std::unique_ptr<T> pmake_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class pmempool
{
private:
	struct block
	{
		block() : m_num_alloc(0), m_free(0), cur_ptr(nullptr), data(nullptr) { }
		int m_num_alloc;
		int m_free;
		char *cur_ptr;
		char *data;
	};

	int new_block();

	struct info
	{
		info() : m_block(0) { }
		size_t m_block;
	};

public:
	pmempool(int min_alloc, int min_align);
	~pmempool();

	void *alloc(size_t size);
	void free(void *ptr);

	int m_min_alloc;
	int m_min_align;

	std::vector<block> m_blocks;
};


#endif /* NLCONFIG_H_ */
