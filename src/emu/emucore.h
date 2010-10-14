/***************************************************************************

    emucore.h

    General core utilities and macros used throughout the emulator.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMUCORE_H__
#define __EMUCORE_H__

// standard C includes
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// some cleanups for Solaris for things defined in stdlib.h
#ifdef SDLMAME_SOLARIS
#undef si_status
#undef WWORD
#endif

// standard C++ includes
#include <exception>
#include <typeinfo>

// core system includes
#include "osdcomm.h"
#include "emualloc.h"
#include "corestr.h"
#include "astring.h"
#include "bitmap.h"
#include "tagmap.h"



//**************************************************************************
//  COMPILER-SPECIFIC NASTINESS
//**************************************************************************

// Suppress warnings about redefining the macro 'PPC' on LinuxPPC.
#undef PPC

// Suppress warnings about redefining the macro 'ARM' on ARM.
#undef ARM



//**************************************************************************
//  FUNDAMENTAL TYPES
//**************************************************************************

// genf is a generic function pointer; cast function pointers to this instead of void *
typedef void genf(void);

// FPTR is used to cast a pointer to a scalar
#ifdef PTR64
typedef UINT64 FPTR;
#else
typedef UINT32 FPTR;
#endif

// pen_t is used to represent pixel values in bitmaps
typedef UINT32 pen_t;

// stream_sample_t is used to represent a single sample in a sound stream
typedef INT32 stream_sample_t;

// running_machine is core to pretty much everything
class running_machine;



//**************************************************************************
//  USEFUL COMPOSITE TYPES
//**************************************************************************

// generic_ptr is a union of pointers to various sizes
union generic_ptr
{
	void *		v;
	INT8 *		i8;
	UINT8 *		u8;
	INT16 *		i16;
	UINT16 *	u16;
	INT32 *		i32;
	UINT32 *	u32;
	INT64 *		i64;
	UINT64 *	u64;
};


// PAIR is an endian-safe union useful for representing 32-bit CPU registers
union PAIR
{
#ifdef LSB_FIRST
	struct { UINT8 l,h,h2,h3; } b;
	struct { UINT16 l,h; } w;
	struct { INT8 l,h,h2,h3; } sb;
	struct { INT16 l,h; } sw;
#else
	struct { UINT8 h3,h2,h,l; } b;
	struct { INT8 h3,h2,h,l; } sb;
	struct { UINT16 h,l; } w;
	struct { INT16 h,l; } sw;
#endif
	UINT32 d;
	INT32 sd;
};


// PAIR64 is a 64-bit extension of a PAIR
union PAIR64
{
#ifdef LSB_FIRST
	struct { UINT8 l,h,h2,h3,h4,h5,h6,h7; } b;
	struct { UINT16 l,h,h2,h3; } w;
	struct { UINT32 l,h; } d;
	struct { INT8 l,h,h2,h3,h4,h5,h6,h7; } sb;
	struct { INT16 l,h,h2,h3; } sw;
	struct { INT32 l,h; } sd;
#else
	struct { UINT8 h7,h6,h5,h4,h3,h2,h,l; } b;
	struct { UINT16 h3,h2,h,l; } w;
	struct { UINT32 h,l; } d;
	struct { INT8 h7,h6,h5,h4,h3,h2,h,l; } sb;
	struct { INT16 h3,h2,h,l; } sw;
	struct { INT32 h,l; } sd;
#endif
	UINT64 q;
	INT64 sq;
};



//**************************************************************************
//  COMMON CONSTANTS
//**************************************************************************

// constants for expression endianness
enum endianness_t
{
	ENDIANNESS_LITTLE,
	ENDIANNESS_BIG
};


// declare native endianness to be one or the other
#ifdef LSB_FIRST
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_LITTLE;
#else
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_BIG;
#endif


// M_PI is not part of the C/C++ standards and is not present on
// strict ANSI compilers or when compiling under GCC with -ansi
#ifndef M_PI
#define M_PI    						3.14159265358979323846
#endif


// orientation of bitmaps
#define	ORIENTATION_FLIP_X				0x0001	/* mirror everything in the X direction */
#define	ORIENTATION_FLIP_Y				0x0002	/* mirror everything in the Y direction */
#define ORIENTATION_SWAP_XY				0x0004	/* mirror along the top-left/bottom-right diagonal */

#define	ROT0							0
#define	ROT90							(ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X)	/* rotate clockwise 90 degrees */
#define	ROT180							(ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y)	/* rotate 180 degrees */
#define	ROT270							(ORIENTATION_SWAP_XY | ORIENTATION_FLIP_Y)	/* rotate counter-clockwise 90 degrees */



//**************************************************************************
//  COMMON MACROS
//**************************************************************************

// macro for defining a copy constructor and assignment operator to prevent copying
#define DISABLE_COPYING(_Type) \
private: \
	_Type(const _Type &); \
	_Type &operator=(const _Type &) \


// macro for declaring enumerator operators that increment/decrement like plain old C
#define DECLARE_ENUM_OPERATORS(_Type) \
inline void operator++(_Type &value) { value = (_Type)((int)value + 1); } \
inline void operator++(_Type &value, int) { value = (_Type)((int)value + 1); } \
inline void operator--(_Type &value) { value = (_Type)((int)value - 1); } \
inline void operator--(_Type &value, int) { value = (_Type)((int)value - 1); }


// standard assertion macros
#undef assert
#undef assert_always

#ifdef MAME_DEBUG
#define assert(x)				do { if (!(x)) throw emu_fatalerror("assert: %s:%d: %s", __FILE__, __LINE__, #x); } while (0)
#define assert_always(x, msg)	do { if (!(x)) throw emu_fatalerror("Fatal error: %s\nCaused by assert: %s:%d: %s", msg, __FILE__, __LINE__, #x); } while (0)
#else
#define assert(x)				do { } while (0)
#define assert_always(x, msg)	do { if (!(x)) throw emu_fatalerror("Fatal error: %s (%s:%d)", msg, __FILE__, __LINE__); } while (0)
#endif


// map mame_* helpers to core_* helpers */
#define mame_stricmp		core_stricmp
#define mame_strnicmp		core_strnicmp
#define mame_strdup			core_strdup
#define mame_strwildcmp		core_strwildcmp


// prevent the use of rand() -- use mame_rand() instead
#define rand __error_use_mame_rand_instead__


// macros to convert radians to degrees and degrees to radians
#define RADIAN_TO_DEGREE(x)   ((180.0 / M_PI) * (x))
#define DEGREE_TO_RADIAN(x)   ((M_PI / 180.0) * (x))


// endian-based value: first value is if 'endian' is little-endian, second is if 'endian' is big-endian
#define ENDIAN_VALUE_LE_BE(endian,leval,beval)	(((endian) == ENDIANNESS_LITTLE) ? (leval) : (beval))

// endian-based value: first value is if native endianness is little-endian, second is if native is big-endian
#define NATIVE_ENDIAN_VALUE_LE_BE(leval,beval)	ENDIAN_VALUE_LE_BE(ENDIANNESS_NATIVE, leval, beval)

// endian-based value: first value is if 'endian' matches native, second is if 'endian' doesn't match native
#define ENDIAN_VALUE_NE_NNE(endian,leval,beval)	(((endian) == ENDIANNESS_NATIVE) ? (neval) : (nneval))


// useful macros to deal with bit shuffling encryptions
#define BIT(x,n) (((x)>>(n))&1)

#define BITSWAP8(val,B7,B6,B5,B4,B3,B2,B1,B0) \
	((BIT(val,B7) << 7) | (BIT(val,B6) << 6) | (BIT(val,B5) << 5) | (BIT(val,B4) << 4) | \
	 (BIT(val,B3) << 3) | (BIT(val,B2) << 2) | (BIT(val,B1) << 1) | (BIT(val,B0) << 0))

#define BITSWAP16(val,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
	((BIT(val,B15) << 15) | (BIT(val,B14) << 14) | (BIT(val,B13) << 13) | (BIT(val,B12) << 12) | \
	 (BIT(val,B11) << 11) | (BIT(val,B10) << 10) | (BIT(val, B9) <<  9) | (BIT(val, B8) <<  8) | \
	 (BIT(val, B7) <<  7) | (BIT(val, B6) <<  6) | (BIT(val, B5) <<  5) | (BIT(val, B4) <<  4) | \
	 (BIT(val, B3) <<  3) | (BIT(val, B2) <<  2) | (BIT(val, B1) <<  1) | (BIT(val, B0) <<  0))

#define BITSWAP24(val,B23,B22,B21,B20,B19,B18,B17,B16,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
	((BIT(val,B23) << 23) | (BIT(val,B22) << 22) | (BIT(val,B21) << 21) | (BIT(val,B20) << 20) | \
	 (BIT(val,B19) << 19) | (BIT(val,B18) << 18) | (BIT(val,B17) << 17) | (BIT(val,B16) << 16) | \
	 (BIT(val,B15) << 15) | (BIT(val,B14) << 14) | (BIT(val,B13) << 13) | (BIT(val,B12) << 12) | \
	 (BIT(val,B11) << 11) | (BIT(val,B10) << 10) | (BIT(val, B9) <<  9) | (BIT(val, B8) <<  8) | \
	 (BIT(val, B7) <<  7) | (BIT(val, B6) <<  6) | (BIT(val, B5) <<  5) | (BIT(val, B4) <<  4) | \
	 (BIT(val, B3) <<  3) | (BIT(val, B2) <<  2) | (BIT(val, B1) <<  1) | (BIT(val, B0) <<  0))

#define BITSWAP32(val,B31,B30,B29,B28,B27,B26,B25,B24,B23,B22,B21,B20,B19,B18,B17,B16,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
	((BIT(val,B31) << 31) | (BIT(val,B30) << 30) | (BIT(val,B29) << 29) | (BIT(val,B28) << 28) | \
	 (BIT(val,B27) << 27) | (BIT(val,B26) << 26) | (BIT(val,B25) << 25) | (BIT(val,B24) << 24) | \
	 (BIT(val,B23) << 23) | (BIT(val,B22) << 22) | (BIT(val,B21) << 21) | (BIT(val,B20) << 20) | \
	 (BIT(val,B19) << 19) | (BIT(val,B18) << 18) | (BIT(val,B17) << 17) | (BIT(val,B16) << 16) | \
	 (BIT(val,B15) << 15) | (BIT(val,B14) << 14) | (BIT(val,B13) << 13) | (BIT(val,B12) << 12) | \
	 (BIT(val,B11) << 11) | (BIT(val,B10) << 10) | (BIT(val, B9) <<  9) | (BIT(val, B8) <<  8) | \
	 (BIT(val, B7) <<  7) | (BIT(val, B6) <<  6) | (BIT(val, B5) <<  5) | (BIT(val, B4) <<  4) | \
	 (BIT(val, B3) <<  3) | (BIT(val, B2) <<  2) | (BIT(val, B1) <<  1) | (BIT(val, B0) <<  0))



//**************************************************************************
//  EXCEPTION CLASSES
//**************************************************************************

// emu_exception is the base class for all emu-related exceptions
class emu_exception : public std::exception { };


// emu_fatalerror is a generic fatal exception that provides an error string
class emu_fatalerror : public emu_exception
{
public:
	emu_fatalerror(const char *format, ...)
		: code(0)
	{
		va_list ap;
		va_start(ap, format);
		vsprintf(text, format, ap);
		va_end(ap);
		osd_break_into_debugger(text);
	}

	emu_fatalerror(const char *format, va_list ap)
		: code(0)
	{
		vsprintf(text, format, ap);
		osd_break_into_debugger(text);
	}

	emu_fatalerror(int _exitcode, const char *format, va_list ap)
		: code(_exitcode)
	{
		vsprintf(text, format, ap);
	}

	const char *string() const { return text; }
	int exitcode() const { return code; }

private:
	char text[1024];
	int code;
};



//**************************************************************************
//  CASTING TEMPLATES
//**************************************************************************

// template function for casting from a base class to a derived class that is checked
// in debug builds and fast in release builds
template<class _Dest, class _Source>
inline _Dest downcast(_Source *src)
{
	assert(dynamic_cast<_Dest>(src) == src);
	return static_cast<_Dest>(src);
}

template<class _Dest, class _Source>
inline _Dest downcast(_Source &src)
{
	assert(&dynamic_cast<_Dest>(src) == &src);
	return static_cast<_Dest>(src);
}


// template function for cross-casting from one class to another that throws a bad_cast
// exception instead of returning NULL
template<class _Dest, class _Source>
inline _Dest crosscast(_Source *src)
{
	_Dest result = dynamic_cast<_Dest>(src);
	assert(result != NULL);
	if (result == NULL)
		throw std::bad_cast();
	return result;
}



//**************************************************************************
//  COMMON TEMPLATES
//**************************************************************************

// ======================> simple_list

template<class T>
class simple_list
{
	DISABLE_COPYING(simple_list);

	T *m_head;
	T *m_tail;
	resource_pool &m_pool;
	int m_count;

public:
	simple_list(resource_pool &pool = global_resource_pool) :
		m_head(NULL),
		m_tail(NULL),
		m_pool(pool),
		m_count(0) { }

	virtual ~simple_list() { reset(); }

	T *first() const { return m_head; }
	T *last() const { return m_tail; }
	int count() const { return m_count; }

	void reset() { while (m_head != NULL) remove(*m_head); }

	T &prepend(T &object)
	{
		object.m_next = m_head;
		m_head = &object;
		if (m_tail == NULL)
			m_tail = m_head;
		m_count++;
		return object;
	}

	void prepend_list(simple_list<T> &list)
	{
		int count = list.count();
		if (count == 0)
			return;
		T *tail = list.last();
		T *head = list.detach_all();
		tail->m_next = m_head;
		m_head = head;
		if (m_tail == NULL)
			m_tail = tail;
		m_count += count;
	}

	T &append(T &object)
	{
		object.m_next = NULL;
		if (m_tail != NULL)
			m_tail = m_tail->m_next = &object;
		else
			m_tail = m_head = &object;
		m_count++;
		return object;
	}

	void append_list(simple_list<T> &list)
	{
		int count = list.count();
		if (count == 0)
			return;
		T *tail = list.last();
		T *head = list.detach_all();
		if (m_tail != NULL)
			m_tail->m_next = head;
		else
			m_head = head;
		m_tail = tail;
		m_count += count;
	}

	T *detach_head()
	{
		T *result = m_head;
		if (result != NULL)
		{
			m_head = result->m_next;
			m_count--;
			if (m_head == NULL)
				m_tail = NULL;
		}
		return result;
	}

	T &detach(T &object)
	{
		T *prev = NULL;
		for (T *cur = m_head; cur != NULL; prev = cur, cur = cur->m_next)
			if (cur == &object)
			{
				if (prev != NULL)
					prev->m_next = object.m_next;
				else
					m_head = object.m_next;
				if (m_tail == &object)
					m_tail = prev;
				m_count--;
				return object;
			}
		return object;
	}

	T *detach_all()
	{
		T *result = m_head;
		m_head = m_tail = NULL;
		m_count = 0;
		return result;
	}

	void remove(T &object)
	{
		detach(object);
		pool_free(m_pool, &object);
	}

	T *find(int index) const
	{
		for (T *cur = m_head; cur != NULL; cur = cur->m_next)
			if (index-- == 0)
				return cur;
		return NULL;
	}

	int indexof(const T &object) const
	{
		int index = 0;
		for (T *cur = m_head; cur != NULL; cur = cur->m_next)
		{
			if (cur == &object)
				return index;
			index++;
		}
		return -1;
	}
};


// ======================> fixed_allocator

template<class T>
class fixed_allocator
{
	DISABLE_COPYING(fixed_allocator);

public:
	fixed_allocator(resource_pool &pool = global_resource_pool)
		: m_pool(pool),
		  m_freelist(pool) { }

	T *alloc()
	{
		T *result = m_freelist.detach_head();
		if (result == NULL)
			result = m_pool.add_object(new T);
		return result;
	}

	void reclaim(T *item) { if (item != NULL) m_freelist.append(*item); }
	void reclaim(T &item) { m_freelist.append(item); }
	void reclaim_all(simple_list<T> &list) { m_freelist.append_list(list); }

private:
	resource_pool &m_pool;
	simple_list<T> m_freelist;
};


// ======================> tagged_list

template<class T>
class tagged_list
{
	DISABLE_COPYING(tagged_list);

	T *m_head;
	T **m_tailptr;
	tagmap_t<T *> m_map;
	resource_pool &m_pool;

public:
	tagged_list(resource_pool &pool = global_resource_pool) :
		m_head(NULL),
		m_tailptr(&m_head),
		m_pool(pool) { }

	virtual ~tagged_list()
	{
		reset();
	}

	void reset()
	{
		while (m_head != NULL)
			remove(m_head);
	}

	T *first() const { return m_head; }

	int count() const
	{
		int num = 0;
		for (T *cur = m_head; cur != NULL; cur = cur->m_next)
			num++;
		return num;
	}

	int index(T *object) const
	{
		int num = 0;
		for (T *cur = m_head; cur != NULL; cur = cur->m_next)
			if (cur == object)
				return num;
			else
				num++;
		return -1;
	}

	int index(const char *tag) const
	{
		T *object = find(tag);
		return (object != NULL) ? index(object) : -1;
	}

	T *replace(const char *tag, T *object)
	{
		T *existing = find(tag);
		if (existing == NULL)
			return append(tag, object);

		for (T **objectptr = &m_head; *objectptr != NULL; objectptr = &(*objectptr)->m_next)
			if (*objectptr == existing)
			{
				*objectptr = object;
				object->m_next = existing->m_next;
				if (m_tailptr == &existing->m_next)
					m_tailptr = &object->m_next;
				m_map.remove(existing);
				pool_free(m_pool, existing);
				if (m_map.add_unique_hash(tag, object, false) != TMERR_NONE)
					throw emu_fatalerror("Error replacing object named '%s'", tag);
				break;
			}
		return object;
	}

	T *append(const char *tag, T *object, bool replace_if_duplicate = false)
	{
		if (m_map.add_unique_hash(tag, object, false) != TMERR_NONE)
			throw emu_fatalerror("Error adding object named '%s'", tag);
		*m_tailptr = object;
		object->m_next = NULL;
		m_tailptr = &object->m_next;
		return object;
	}

	void detach(T *object)
	{
		for (T **objectptr = &m_head; *objectptr != NULL; objectptr = &(*objectptr)->m_next)
			if (*objectptr == object)
			{
				*objectptr = object->m_next;
				if (m_tailptr == &object->m_next)
					m_tailptr = objectptr;
				m_map.remove(object);
				return;
			}
	}

	void remove(T *object)
	{
		detach(object);
		pool_free(m_pool, object);
	}

	void remove(const char *tag)
	{
		T *object = find(tag);
		if (object != NULL)
			remove(object);
	}

	T *find(const char *tag) const
	{
		return m_map.find_hash_only(tag);
	}

	T *find(int index) const
	{
		for (T *cur = m_head; cur != NULL; cur = cur->m_next)
			if (index-- == 0)
				return cur;
		return NULL;
	}
};



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

DECL_NORETURN void fatalerror(const char *format, ...) ATTR_PRINTF(1,2) ATTR_NORETURN;
DECL_NORETURN void fatalerror_exitcode(running_machine *machine, int exitcode, const char *format, ...) ATTR_PRINTF(3,4) ATTR_NORETURN;

inline void fatalerror(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	throw emu_fatalerror(format, ap);
	va_end(ap);
}

inline void fatalerror_exitcode(running_machine *machine, int exitcode, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	throw emu_fatalerror(exitcode, format, ap);
	va_end(ap);
}



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

// population count
inline int popcount(UINT32 val)
{
	int count;

	for (count = 0; val != 0; count++)
		val &= val - 1;
	return count;
}


// convert a series of 32 bits into a float
inline float u2f(UINT32 v)
{
	union {
		float ff;
		UINT32 vv;
	} u;
	u.vv = v;
	return u.ff;
}


// convert a float into a series of 32 bits
inline UINT32 f2u(float f)
{
	union {
		float ff;
		UINT32 vv;
	} u;
	u.ff = f;
	return u.vv;
}


// convert a series of 64 bits into a double
inline double u2d(UINT64 v)
{
	union {
		double dd;
		UINT64 vv;
	} u;
	u.vv = v;
	return u.dd;
}


// convert a double into a series of 64 bits
inline UINT64 d2u(double d)
{
	union {
		double dd;
		UINT64 vv;
	} u;
	u.dd = d;
	return u.vv;
}

#endif	/* __EMUCORE_H__ */
