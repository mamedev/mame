// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * palloc.h
 *
 */

#ifndef PALLOC_H_
#define PALLOC_H_

#include <exception>

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

class pmemory_pool;

extern pmemory_pool *ppool;

void *palloc_raw(const size_t size);
void pfree_raw(void *p);

void* operator new(std::size_t size, pmemory_pool *pool) throw (std::bad_alloc);

void operator delete(void *ptr, pmemory_pool *pool);

template<typename T>
inline void pfree_t(T *p)
{
	p->~T();
	pfree_raw(p);
}

template <typename T>
inline T *palloc_array_t(size_t N)
{
	char *buf = reinterpret_cast<char *>(palloc_raw(N * sizeof(T) + 64*2));
	size_t *s = reinterpret_cast<size_t *>(buf);
	*s = N;
	buf += 64;
	T *p = reinterpret_cast<T *>(buf);
	for (size_t i = 0; i < N; i++)
		new(reinterpret_cast<void *>(&p[i])) T();
	return p;
}

template <typename T>
inline void pfree_array_t(T *p)
{
	char *buf = reinterpret_cast<char *>(p);
	buf -= 64;
	size_t *s = reinterpret_cast<size_t *>(buf);
	size_t N = *s;
	while (N > 0)
	{
			p->~T();
			p++;
			N--;
	}
	pfree_raw(s);
}

#define palloc(T)             new(ppool) T
#define pfree(_ptr)           pfree_t(_ptr)

#if 1
#define palloc_array(T, N)    palloc_array_t<T>(N)
#define pfree_array(_ptr)     pfree_array_t(_ptr)
#else
#define palloc_array(T, N)    new T[N]
#define pfree_array(_ptr)     delete[] _ptr
#endif
#else
#include "corealloc.h"

#define ATTR_ALIGN

#define palloc(T)             global_alloc(T)
#define pfree(_ptr)           global_free(_ptr)

#define palloc_array(T, N)    global_alloc_array(T, N)
#define pfree_array(_ptr)     global_free_array(_ptr)

#endif

#endif /* NLCONFIG_H_ */
