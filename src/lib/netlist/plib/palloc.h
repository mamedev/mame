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

#else

#define ATTR_ALIGN

#endif

PLIB_NAMESPACE_START()

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

template<typename T, typename... Args>
std::unique_ptr<T> pmake_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename BC, typename DC, typename... Args>
static std::unique_ptr<BC> make_unique_base(Args&&... args)
{
	std::unique_ptr<BC> ret(new DC(std::forward<Args>(args)...));
	return ret;
}

template <typename SC>
class powned_ptr
{
private:
	powned_ptr()
	: m_ptr(nullptr), m_is_owned(true) { }
public:
	powned_ptr(SC *p, bool owned)
	: m_ptr(p), m_is_owned(owned)
	{ }
	powned_ptr(const powned_ptr &r) = delete;
	powned_ptr & operator =(const powned_ptr &r) = delete;

	powned_ptr(powned_ptr &&r)
	{
		m_is_owned = r.m_is_owned;
		m_ptr = r.m_ptr;
		r.m_is_owned = false;
		r.m_ptr = nullptr;
	}

	template<typename DC>
	powned_ptr(powned_ptr<DC> &&r)
	{
		SC *dest_ptr = &dynamic_cast<SC &>(*r.get());
		bool o = r.is_owned();
		r.release();
		m_is_owned = o;
		m_ptr = dest_ptr;
	}

	~powned_ptr()
	{
		if (m_is_owned)
			delete m_ptr;
	}
	template<typename DC, typename... Args>
	static powned_ptr Create(Args&&... args)
	{
		powned_ptr a;
		DC *x = new DC(std::forward<Args>(args)...);
		a.m_ptr = static_cast<SC *>(x);
		return a;
	}

	template<typename... Args>
	static powned_ptr Create(Args&&... args)
	{
		powned_ptr a;
		a.m_ptr = new SC(std::forward<Args>(args)...);
		return a;
	}
	void release()
	{
		m_is_owned = false;
		m_ptr = nullptr;
	}

	bool is_owned() const { return m_is_owned; }

	template<typename DC>
	powned_ptr<DC> & operator =(powned_ptr<DC> &r)
	{
		m_is_owned = r.m_is_owned;
		m_ptr = r.m_ptr;
		r.m_is_owned = false;
		r.m_ptr = nullptr;
		return *this;
	}
	SC * operator ->() { return m_ptr; }
	SC & operator *() { return *m_ptr; }
	SC * get() const { return m_ptr; }
private:
	SC *m_ptr;
	bool m_is_owned;
};

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

PLIB_NAMESPACE_END()

#endif /* PALLOC_H_ */
