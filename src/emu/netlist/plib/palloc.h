// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * palloc.h
 *
 */

#ifndef PALLOC_H_
#define PALLOC_H_

#include "pconfig.h"

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


void *palloc_raw(const size_t size);
void pfree_raw(void *p);

template<typename T>
inline T *palloc_t()
{
	void *p = palloc_raw(sizeof(T));
	return new (p) T();
}

template<typename T, typename P1>
inline T *palloc_t(P1 p1)
{
	void *p = palloc_raw(sizeof(T));
	return new (p) T(p1);
}

template<typename T, typename P1, typename P2>
inline T *palloc_t(P1 p1, P2 p2)
{
	void *p = palloc_raw(sizeof(T));
	return new (p) T(p1, p2);
}

template<typename T, typename P1, typename P2, typename P3>
inline T *palloc_t(P1 p1, P2 p2, P3 p3)
{
	void *p = palloc_raw(sizeof(T));
	return new (p) T(p1, p2, p3);
}

template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
inline T *palloc_t(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
{
	void *p = palloc_raw(sizeof(T));
	return new (p) T(p1, p2, p3, p4, p5, p6, p7);
}

template<typename T>
inline void pfree_t(T *p)
{
	p->~T();
	pfree_raw(p);
	//delete p;
}

template <typename T>
inline T *palloc_array_t(size_t N)
{
	return new T[N];
}

template <typename T>
inline void pfree_array_t(T *p)
{
	delete[] p;
}

#if 1
#define palloc(T, ...)        palloc_t<T>(__VA_ARGS__)
#define pfree(_ptr)           pfree_t(_ptr)
#else
#define palloc(T, ...)        new T(__VA_ARGS__)
#define pfree(_ptr)           delete(_ptr)
#endif
#define palloc_array(T, N)    palloc_array_t<T>(N)
#define pfree_array(_ptr)     pfree_array_t(_ptr)

#else

#define ATTR_ALIGN

#include "corealloc.h"
#define palloc(T, ...)        global_alloc(T(__VA_ARGS__))
#define pfree(_ptr)           global_free(_ptr)

#define palloc_array(T, N)    global_alloc_array(T, N)
#define pfree_array(_ptr)     global_free_array(_ptr)

#endif

#endif /* NLCONFIG_H_ */
