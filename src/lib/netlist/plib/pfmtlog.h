// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file pfmtlog.h
///

#ifndef PFMTLOG_H_
#define PFMTLOG_H_

#include "penum.h"
#include "pgsl.h"
#include "ppmf.h"
#include "pstring.h"
#include "ptypes.h"

#include <limits>
#include <locale>
#include <sstream>

#define PERRMSGV(name, narg, str) \
	struct name : public plib::perrmsg \
	{ \
		template<typename... Args> explicit name(Args&&... args) \
		: plib::perrmsg(str, std::forward<Args>(args)...) \
		{ static_assert((narg) == sizeof...(args), "Argument count mismatch"); } \
	};

namespace plib {

	PENUM(plog_level,
		DEBUG,
		VERBOSE,
		INFO,
		WARNING,
		ERROR,
		FATAL)

	template <typename T>
	struct ptype_traits_base
	{
		static constexpr const bool is_signed = std::numeric_limits<T>::is_signed;
		static char32_t fmt_spec() { return 'u'; }
		static void streamify(std::ostream &s, const T &v)
		{
			s << v;
		}
	};

	#if (PUSE_FLOAT128)
	template <>
	struct ptype_traits_base<FLOAT128>
	{
		// FIXME: need native support at some time
		static constexpr const bool is_signed = true;
		static char32_t fmt_spec() { return 'f'; }
		static void streamify(std::ostream &s, const FLOAT128 &v)
		{
			s << narrow_cast<long double>(v);
		}
	};
	#endif

	template <typename T>
	struct ptype_traits;

	template<>
	struct ptype_traits<compile_info::int128_type>
	{
		// FIXME: need native support at some time
		static constexpr const bool is_signed = true;
		static char32_t fmt_spec() { return 'd'; }
		template <typename T, typename = std::enable_if_t<plib::is_arithmetic<T>::value>>
		static void streamify(std::ostream &s, const T &v)
		{
			s << narrow_cast<long long>(v);
		}
	};

	template<>
	struct ptype_traits<bool> : ptype_traits_base<bool>
	{
	};

	template<>
	struct ptype_traits<char> : ptype_traits_base<char>
	{
		static char32_t fmt_spec() { return is_signed ? 'd' : 'u'; }
	};

	template<>
	struct ptype_traits<short> : ptype_traits_base<short>
	{
		static char32_t fmt_spec() { return 'd'; }
	};

	template<>
	struct ptype_traits<int> : ptype_traits_base<int>
	{
		static char32_t fmt_spec() { return 'd'; }
	};

	template<>
	struct ptype_traits<long> : ptype_traits_base<long>
	{
		static char32_t fmt_spec() { return 'd'; }
	};

	template<>
	struct ptype_traits<long long> : ptype_traits_base<long long>
	{
		static char32_t fmt_spec() { return 'd'; }
	};

	template<>
	struct ptype_traits<signed char> : ptype_traits_base<signed char>
	{
		static char32_t fmt_spec() { return 'd'; }
	};

	template<>
	struct ptype_traits<unsigned char> : ptype_traits_base<unsigned char>
	{
		static char32_t fmt_spec() { return 'u'; }
	};

	template<>
	struct ptype_traits<unsigned short> : ptype_traits_base<unsigned short>
	{
		static char32_t fmt_spec() { return 'u'; }
	};

	template<>
	struct ptype_traits<unsigned int> : ptype_traits_base<unsigned int>
	{
		static char32_t fmt_spec() { return 'u'; }
	};

	template<>
	struct ptype_traits<unsigned long> : ptype_traits_base<unsigned long>
	{
		static char32_t fmt_spec() { return 'u'; }
	};

	template<>
	struct ptype_traits<unsigned long long> : ptype_traits_base<unsigned long long>
	{
		static char32_t fmt_spec() { return 'u'; }
	};

	template<>
	struct ptype_traits<float> : ptype_traits_base<float>
	{
		static char32_t fmt_spec() { return 'f'; }
	};

	template<>
	struct ptype_traits<double> : ptype_traits_base<double>
	{
		static char32_t fmt_spec() { return 'f'; }
	};

	template<>
	struct ptype_traits<long double> : ptype_traits_base<long double>
	{
		static char32_t fmt_spec() { return 'f'; }
	};

	#if (PUSE_FLOAT128)
	template<>
	struct ptype_traits<FLOAT128> : ptype_traits_base<FLOAT128>
	{
		static char32_t fmt_spec() { return 'f'; }
	};
	#endif


	template<>
	struct ptype_traits<char *> : ptype_traits_base<char *>
	{
		static char32_t fmt_spec() { return 's'; }
	};

	template<>
	struct ptype_traits<const char *> : ptype_traits_base<const char *>
	{
		static char32_t fmt_spec() { return 's'; }
	};

	template<>
	struct ptype_traits<const char16_t *> : ptype_traits_base<const char16_t *>
	{
		static char32_t fmt_spec() { return 's'; }
		static void streamify(std::ostream &s, const char16_t *v)
		{
			const putf16string su16(v);
			s << putf8string(su16).c_str();
		}
	};

	template<>
	struct ptype_traits<const char32_t *> : ptype_traits_base<const char32_t *>
	{
		static char32_t fmt_spec() { return 's'; }
		static void streamify(std::ostream &s, const char32_t *v)
		{
			const putf32string su32(v);
			s << putf8string(su32).c_str();
		}
	};

	template<>
	struct ptype_traits<std::string> : ptype_traits_base<std::string>
	{
		static char32_t fmt_spec() { return 's'; }
	};

	template<>
	struct ptype_traits<putf8string> : ptype_traits_base<putf8string>
	{
		static char32_t fmt_spec() { return 's'; }
	};

	template<>
	struct ptype_traits<putf16string> : ptype_traits_base<putf16string>
	{
		static char32_t fmt_spec() { return 's'; }
		static void streamify(std::ostream &s, const putf16string &v)
		{
			s << putf8string(v).c_str();
		}
	};

	template<>
	struct ptype_traits<const void *> : ptype_traits_base<const void *>
	{
		static char32_t fmt_spec() { return 'p'; }
	};

	class pfmt
	{
	public:
		explicit pfmt(const pstring &fmt)
		: m_str(fmt), m_locale(std::locale::classic()), m_arg(0)
		{
		}
		explicit pfmt(const std::locale &loc, const pstring &fmt)
		: m_str(fmt), m_locale(loc), m_arg(0)
		{
		}

		PCOPYASSIGNMOVE(pfmt, default)

		~pfmt() noexcept = default;

		operator pstring() const { return m_str; }

		template <typename T>
		std::enable_if_t<plib::is_floating_point<T>::value, pfmt &>
		f(const T &x) {return format_element('f', x);  }

		template <typename T>
		std::enable_if_t<plib::is_floating_point<T>::value, pfmt &>
		e(const T &x) {return format_element('e', x);  }

		template <typename T>
		std::enable_if_t<plib::is_floating_point<T>::value, pfmt &>
		g(const T &x) {return format_element('g', x);  }

		pfmt &operator ()(const void *x) {return format_element('p', x);  }
		pfmt &operator ()(const pstring &x) {return format_element('s', x.c_str() );  }

		template<typename T>
		pfmt &operator ()(const T &x)
		{
			return format_element(x);
		}

		template<typename T>
		pfmt &operator ()(const T *x)
		{
			return format_element(x);
		}

		pfmt &operator ()()
		{
			return *this;
		}

		template<typename X, typename Y, typename... Args>
		pfmt &operator()(X&& x, Y && y, Args&&... args)
		{
			return ((*this)(std::forward<X>(x)))(std::forward<Y>(y), std::forward<Args>(args)...);
		}

		template<typename T>
		std::enable_if_t<plib::is_integral<T>::value, pfmt &>
		x(const T &x)
		{
			return format_element('x', x);
		}

		template<typename T>
		std::enable_if_t<plib::is_integral<T>::value, pfmt &>
		o(const T &x)
		{
			return format_element('o', x);
		}

		friend std::ostream& operator<<(std::ostream &ostrm, const pfmt &fmt)
		{
			ostrm << putf8string(fmt.m_str);
			return ostrm;
		}

	protected:

		struct rtype
		{
			rtype() : ret(0), p(0), sl(0) {}
			int ret;
			pstring::size_type p;
			pstring::size_type sl;
		};
		rtype setfmt(std::stringstream &strm, char32_t cfmt_spec);

		template <typename T>
		pfmt &format_element(T &&v)
		{
			return format_element(ptype_traits<typename std::decay<T>::type>::fmt_spec(), std::forward<T>(v));
		}

		template <typename T>
		pfmt &format_element(const char32_t cfmt_spec, T &&v)
		{
			rtype ret;

			m_arg++;

			do {
				std::stringstream strm;
				strm.imbue(m_locale);
				ret = setfmt(strm, cfmt_spec);
				if (ret.ret>=0)
				{
					ptype_traits<typename std::decay<T>::type>::streamify(strm, std::forward<T>(v));
					const pstring ps(putf8string(strm.str()));
					m_str = m_str.substr(0, ret.p) + ps + m_str.substr(ret.p + ret.sl);
				}
			} while (ret.ret == 1);

			return *this;
		}


	private:

		pstring m_str;
		std::locale m_locale;
		unsigned m_arg;
	};

	template <class T, bool build_enabled = true>
	class pfmt_writer_t
	{
	public:
		explicit pfmt_writer_t() : m_enabled(true)  { }

		PCOPYASSIGNMOVE(pfmt_writer_t, delete)

		// runtime enable
		template<bool enabled, typename... Args>
		void log(const pstring & fmt, Args&&... args) const noexcept
		{
			if (build_enabled && enabled && m_enabled)
			{
				pfmt pf(fmt);
				dynamic_cast<T &>(*this).vdowrite(xlog(pf, std::forward<Args>(args)...));
			}
		}

		template<typename... Args>
		void operator ()(const pstring &fmt, Args&&... args) const noexcept
		{
			if (build_enabled && m_enabled)
			{
				pfmt pf(fmt);
				static_cast<const T &>(*this).vdowrite(xlog(pf, std::forward<Args>(args)...));
			}
		}

		void set_enabled(const bool v)
		{
			m_enabled = v;
		}

		bool is_enabled() const { return m_enabled; }

	protected:
		~pfmt_writer_t() noexcept = default;

	private:
		pfmt &xlog(pfmt &fmt) const { return fmt; }

		template<typename X, typename... Args>
		pfmt &xlog(pfmt &fmt, X&& x, Args&&... args) const
		{
			return xlog(fmt(std::forward<X>(x)), std::forward<Args>(args)...);
		}

		bool m_enabled;

	};

	using plog_delegate = plib::pmfp<void (plog_level, const pstring &)>;

	template <plog_level::E L, bool build_enabled = true>
	class plog_channel : public pfmt_writer_t<plog_channel<L, build_enabled>, build_enabled>
	{
		friend class pfmt_writer_t<plog_channel<L, build_enabled>, build_enabled>;
	public:
		explicit plog_channel(plog_delegate logger)
		: pfmt_writer_t<plog_channel, build_enabled>()
		, m_logger(logger) { }

		PCOPYASSIGNMOVE(plog_channel, delete)

		~plog_channel() noexcept = default;

	protected:
		void vdowrite(const pstring &ls) const noexcept
		{
			m_logger(L, ls);
		}

	private:
		plog_delegate m_logger;
	};

	template<bool debug_enabled>
	class plog_base
	{
	public:

		explicit plog_base(plog_delegate logger)
		: debug(logger),
			info(logger),
			verbose(logger),
			warning(logger),
			error(logger),
			fatal(logger)
		{}

		PCOPYASSIGNMOVE(plog_base, delete)
		virtual ~plog_base() noexcept = default;

		plog_channel<plog_level::DEBUG, debug_enabled> debug;
		plog_channel<plog_level::INFO> info;
		plog_channel<plog_level::VERBOSE> verbose;
		plog_channel<plog_level::WARNING> warning;
		plog_channel<plog_level::ERROR> error;
		plog_channel<plog_level::FATAL> fatal;
	};

	struct perrmsg
	{
		template<std::size_t N, typename... Args>
		explicit perrmsg(const char (&fmt)[N], Args&&... args) // NOLINT(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
		: m_msg(plib::pfmt(fmt)(std::forward<Args>(args)...))
		{ }
		operator pstring const & () const noexcept { return m_msg; }
		const pstring & operator ()() const noexcept { return m_msg; }
	private:
		pstring m_msg;
	};


} // namespace plib

template<typename T>
plib::pfmt& operator<<(plib::pfmt &p, T&& val) { return p(std::forward<T>(val)); }

#endif // PFMT_LOG_H_
