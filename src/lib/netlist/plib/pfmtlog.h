// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pfmtlog.h
 */

#ifndef _PFMT_H_
#define _PFMT_H_

//#include <cstdarg>
//#include <cstddef>

#include "pconfig.h"
#include "pstring.h"
#include "ptypes.h"

template <typename T>
struct ptype_treats
{
};

template<>
struct ptype_treats<char>
{
	static short cast(char x) { return x; }
	static const bool is_signed = true;
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_treats<short>
{
	static short cast(short x) { return x; }
	static const bool is_signed = true;
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_treats<int>
{
	static int cast(int x) { return x; }
	static const bool is_signed = true;
	static const char *size_specifier() { return ""; }
};

template<>
struct ptype_treats<long>
{
	static long cast(long x) { return x; }
	static const bool is_signed = true;
	static const char *size_specifier() { return "l"; }
};

template<>
struct ptype_treats<long long>
{
	static long long cast(long long x) { return x; }
	static const bool is_signed = true;
	static const char *size_specifier() { return "ll"; }
};

template<>
struct ptype_treats<unsigned char>
{
	static unsigned short cast(unsigned char x) { return x; }
	static const bool is_signed = false;
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_treats<unsigned short>
{
	static unsigned short cast(unsigned short x) { return x; }
	static const bool is_signed = false;
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_treats<unsigned int>
{
	static unsigned int cast(unsigned int x) { return x; }
	static const bool is_signed = false;
	static const char *size_specifier() { return ""; }
};

template<>
struct ptype_treats<unsigned long>
{
	static unsigned long cast(unsigned long x) { return x; }
	static const bool is_signed = false;
	static const char *size_specifier() { return "l"; }
};

template<>
struct ptype_treats<unsigned long long>
{
	static unsigned long long cast(unsigned long long x) { return x; }
	static const bool is_signed = false;
	static const char *size_specifier() { return "ll"; }
};

template <typename P>
class pformat_base
{
public:

	virtual ~pformat_base() { }

	ATTR_COLD P &operator ()(const double x, const char *f = "") { format_element(f, "", "f", x); return static_cast<P &>(*this); }
	ATTR_COLD P &          e(const double x, const char *f = "") { format_element(f, "", "e", x); return static_cast<P &>(*this);  }
	ATTR_COLD P &          g(const double x, const char *f = "") { format_element(f, "", "g", x); return static_cast<P &>(*this);  }

	ATTR_COLD P &operator ()(const char *x, const char *f = "") { format_element(f, "", "s", x); return static_cast<P &>(*this);  }
	ATTR_COLD P &operator ()(char *x, const char *f = "") { format_element(f, "", "s", x); return static_cast<P &>(*this);  }
	ATTR_COLD P &operator ()(const void *x, const char *f = "") { format_element(f, "", "p", x); return static_cast<P &>(*this);  }
	ATTR_COLD P &operator ()(const pstring &x, const char *f = "") { format_element(f, "", "s", x.cstr() ); return static_cast<P &>(*this);  }

	template<typename T>
	ATTR_COLD P &operator ()(const T x, const char *f = "")
	{
		if (ptype_treats<T>::is_signed)
			format_element(f, ptype_treats<T>::size_specifier(), "d", ptype_treats<T>::cast(x));
		else
			format_element(f, ptype_treats<T>::size_specifier(), "u", ptype_treats<T>::cast(x));
		return static_cast<P &>(*this);
	}

	template<typename T>
	ATTR_COLD P &x(const T x, const char *f = "")
	{
		format_element(f, ptype_treats<T>::size_specifier(), "x", x);
		return static_cast<P &>(*this);
	}

	template<typename T>
	ATTR_COLD P &o(const T x, const char *f = "")
	{
		format_element(f, ptype_treats<T>::size_specifier(), "o", x);
		return static_cast<P &>(*this);
	}

protected:

	virtual void format_element(const char *f, const char *l, const char *fmt_spec, ...) = 0;

};

class pfmt : public pformat_base<pfmt>
{
public:
	pfmt(const pstring &fmt);
	pfmt(const char *fmt);
	virtual ~pfmt();

	operator pstring() const { return m_str; }

	const char *cstr() { return m_str; }


protected:
	void format_element(const char *f, const char *l, const char *fmt_spec, ...) override;

private:

	char *m_str;
	char m_str_buf[256];
	unsigned m_allocated;
	unsigned m_arg;
};

P_ENUM(plog_level,
	DEBUG,
	INFO,
	VERBOSE,
	WARNING,
	ERROR,
	FATAL)

class plog_dispatch_intf;

template <bool build_enabled = true>
class pfmt_writer_t
{
public:
	pfmt_writer_t() : m_enabled(true)  { }
	virtual ~pfmt_writer_t() { }

	ATTR_COLD void operator ()(const char *fmt) const
	{
		if (build_enabled && m_enabled) vdowrite(fmt);
	}

	template<typename T1>
	ATTR_COLD void operator ()(const char *fmt, const T1 &v1) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1));
	}

	template<typename T1, typename T2>
	ATTR_COLD void operator ()(const char *fmt, const T1 &v1, const T2 &v2) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1)(v2));
	}

	template<typename T1, typename T2, typename T3>
	ATTR_COLD void operator ()(const char *fmt, const T1 &v1, const T2 &v2, const T3 &v3) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1)(v2)(v3));
	}

	template<typename T1, typename T2, typename T3, typename T4>
	ATTR_COLD void operator ()(const char *fmt, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1)(v2)(v3)(v4));
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	ATTR_COLD void operator ()(const char *fmt, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1)(v2)(v3)(v4)(v5));
	}

	void set_enabled(const bool v)
	{
		m_enabled = v;
	}

	bool is_enabled() const { return m_enabled; }

protected:
	virtual void vdowrite(const pstring &ls) const {}

private:
	bool m_enabled;

};

template <plog_level::e L, bool build_enabled = true>
class plog_channel : public pfmt_writer_t<build_enabled>
{
public:
	plog_channel(plog_dispatch_intf *b) : pfmt_writer_t<build_enabled>(),  m_base(b) { }
	virtual ~plog_channel() { }

protected:
	virtual void vdowrite(const pstring &ls) const override;

private:
	plog_dispatch_intf *m_base;
};

class plog_dispatch_intf
{
	template<plog_level::e, bool> friend class plog_channel;

public:
	virtual ~plog_dispatch_intf() { }
protected:
	virtual void vlog(const plog_level &l, const pstring &ls) const = 0;
};

template<bool debug_enabled>
class plog_base
{
public:

	plog_base(plog_dispatch_intf *proxy)
	: debug(proxy),
		info(proxy),
		verbose(proxy),
		warning(proxy),
		error(proxy),
		fatal(proxy)
	{}
	virtual ~plog_base() {};

	plog_channel<plog_level::DEBUG, debug_enabled> debug;
	plog_channel<plog_level::INFO> info;
	plog_channel<plog_level::VERBOSE> verbose;
	plog_channel<plog_level::WARNING> warning;
	plog_channel<plog_level::ERROR> error;
	plog_channel<plog_level::FATAL> fatal;
};


template <plog_level::e L, bool build_enabled>
void plog_channel<L, build_enabled>::vdowrite(const pstring &ls) const
{
	m_base->vlog(L, ls);
}

#endif /* _PSTRING_H_ */
