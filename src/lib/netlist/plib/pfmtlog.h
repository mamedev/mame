// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pfmtlog.h
 */

#ifndef PFMT_H_
#define PFMT_H_

#include "pstring.h"
#include "ptypes.h"

#include <limits>

namespace plib {

P_ENUM(plog_level,
	DEBUG,
	INFO,
	VERBOSE,
	WARNING,
	ERROR,
	FATAL)

template <typename T>
struct ptype_traits_base
{
	static T cast(T x) { return x; }
	static const bool is_signed = std::numeric_limits<T>::is_signed;
};

template <>
struct ptype_traits_base<bool>
{
	static unsigned int cast(bool x) { return static_cast<unsigned int>(x); }
	static const bool is_signed = std::numeric_limits<bool>::is_signed;
};

template <typename T>
struct ptype_traits;

template<>
struct ptype_traits<bool> : ptype_traits_base<bool>
{
	static const char *size_specifier() { return ""; }
};

template<>
struct ptype_traits<char> : ptype_traits_base<char>
{
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_traits<short> : ptype_traits_base<short>
{
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_traits<int> : ptype_traits_base<int>
{
	static const char *size_specifier() { return ""; }
};

template<>
struct ptype_traits<long> : ptype_traits_base<long>
{
	static const char *size_specifier() { return "l"; }
};

template<>
struct ptype_traits<long long> : ptype_traits_base<long long>
{
	static const char *size_specifier() { return "ll"; }
};

template<>
struct ptype_traits<signed char> : ptype_traits_base<signed char>
{
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_traits<unsigned char> : ptype_traits_base<unsigned char>
{
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_traits<unsigned short> : ptype_traits_base<unsigned short>
{
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_traits<unsigned int> : ptype_traits_base<unsigned int>
{
	static const char *size_specifier() { return ""; }
};

template<>
struct ptype_traits<unsigned long> : ptype_traits_base<unsigned long>
{
	static const char *size_specifier() { return "l"; }
};

template<>
struct ptype_traits<unsigned long long> : ptype_traits_base<unsigned long long>
{
	static const char *size_specifier() { return "ll"; }
};

template <typename P>
class pformat_base
{
public:

	virtual ~pformat_base() { }

	P &operator ()(const double x, const char *f = "") { format_element(f, "", "f", x); return static_cast<P &>(*this); }
	P &          e(const double x, const char *f = "") { format_element(f, "", "e", x); return static_cast<P &>(*this);  }
	P &          g(const double x, const char *f = "") { format_element(f, "", "g", x); return static_cast<P &>(*this);  }

	P &operator ()(const float x, const char *f = "") { format_element(f, "", "f", x); return static_cast<P &>(*this); }
	P &          e(const float x, const char *f = "") { format_element(f, "", "e", x); return static_cast<P &>(*this);  }
	P &          g(const float x, const char *f = "") { format_element(f, "", "g", x); return static_cast<P &>(*this);  }

	P &operator ()(const char *x, const char *f = "") { format_element(f, "", "s", x); return static_cast<P &>(*this);  }
	P &operator ()(char *x, const char *f = "") { format_element(f, "", "s", x); return static_cast<P &>(*this);  }
	P &operator ()(const void *x, const char *f = "") { format_element(f, "", "p", x); return static_cast<P &>(*this);  }
	P &operator ()(const pstring &x, const char *f = "") { format_element(f, "", "s", x.c_str() ); return static_cast<P &>(*this);  }

	template<typename T>
	P &operator ()(const T x, const char *f = "")
	{
		if (ptype_traits<T>::is_signed)
			format_element(f, ptype_traits<T>::size_specifier(), "d", ptype_traits<T>::cast(x));
		else
			format_element(f, ptype_traits<T>::size_specifier(), "u", ptype_traits<T>::cast(x));
		return static_cast<P &>(*this);
	}

	template<typename T>
	P &x(const T x, const char *f = "")
	{
		format_element(f, ptype_traits<T>::size_specifier(), "x", x);
		return static_cast<P &>(*this);
	}

	template<typename T>
	P &o(const T x, const char *f = "")
	{
		format_element(f, ptype_traits<T>::size_specifier(), "o", x);
		return static_cast<P &>(*this);
	}

protected:

	virtual void format_element(const char *f, const char *l, const char *fmt_spec, ...) = 0;

};

class pfmt : public pformat_base<pfmt>
{
public:
	explicit pfmt(const pstring fmt);
	virtual ~pfmt();

	operator pstring() const { return pstring(m_str, pstring::UTF8); }

	const char *c_str() { return m_str; }


protected:
	void format_element(const char *f, const char *l, const char *fmt_spec, ...) override;

private:

	char *m_str;
	char m_str_buf[256];
	std::size_t m_allocated;
	unsigned m_arg;
};

class plog_dispatch_intf;

template <bool build_enabled = true>
class pfmt_writer_t : plib::nocopyassignmove
{
public:
	explicit pfmt_writer_t() : m_enabled(true)  { }
	virtual ~pfmt_writer_t() { }

	/* runtime enable */
	template<bool enabled, typename... Args>
	void log(const pstring fmt, Args&&... args) const
	{
		if (build_enabled && enabled && m_enabled) (*this)(fmt, std::forward<Args>(args)...);
	}

	void operator ()(const pstring fmt) const
	{
		if (build_enabled && m_enabled) vdowrite(fmt);
	}

	template<typename T1>
	void operator ()(const pstring fmt, const T1 &v1) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1));
	}

	template<typename T1, typename T2>
	void operator ()(const pstring fmt, const T1 &v1, const T2 &v2) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1)(v2));
	}

	template<typename T1, typename T2, typename T3>
	void operator ()(const pstring fmt, const T1 &v1, const T2 &v2, const T3 &v3) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1)(v2)(v3));
	}

	template<typename T1, typename T2, typename T3, typename T4>
	void operator ()(const pstring fmt, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1)(v2)(v3)(v4));
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	void operator ()(const pstring fmt, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1)(v2)(v3)(v4)(v5));
	}

	void set_enabled(const bool v)
	{
		m_enabled = v;
	}

	bool is_enabled() const { return m_enabled; }

protected:
	virtual void vdowrite(const pstring &ls) const = 0;

private:
	bool m_enabled;

};

template <plog_level::E L, bool build_enabled = true>
class plog_channel : public pfmt_writer_t<build_enabled>
{
public:
	explicit plog_channel(plog_dispatch_intf *b) : pfmt_writer_t<build_enabled>(), m_base(b) { }
	virtual ~plog_channel() { }

protected:
	virtual void vdowrite(const pstring &ls) const override;

private:
	plog_dispatch_intf *m_base;
};

class plog_dispatch_intf
{
	template<plog_level::E, bool> friend class plog_channel;

public:
	virtual ~plog_dispatch_intf();
protected:
	virtual void vlog(const plog_level &l, const pstring &ls) const = 0;
};

template<bool debug_enabled>
class plog_base
{
public:

	explicit plog_base(plog_dispatch_intf *proxy)
	: debug(proxy),
		info(proxy),
		verbose(proxy),
		warning(proxy),
		error(proxy),
		fatal(proxy)
	{}
	virtual ~plog_base() {}

	plog_channel<plog_level::DEBUG, debug_enabled> debug;
	plog_channel<plog_level::INFO> info;
	plog_channel<plog_level::VERBOSE> verbose;
	plog_channel<plog_level::WARNING> warning;
	plog_channel<plog_level::ERROR> error;
	plog_channel<plog_level::FATAL> fatal;
};


template <plog_level::E L, bool build_enabled>
void plog_channel<L, build_enabled>::vdowrite(const pstring &ls) const
{
	m_base->vlog(L, ls);
}

}


#endif /* PSTRING_H_ */
