// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PTIME_H_
#define PTIME_H_

///
/// \file ptime.h
///

#include "pconfig.h"
#include "pmath.h" // std::floor
#include "ptypes.h"

// ----------------------------------------------------------------------------------------
// netlist_time
// ----------------------------------------------------------------------------------------

namespace plib
{

	template <typename TYPE, TYPE RES>
	struct ptime final
	{
	public:

		using internal_type = TYPE;
		using mult_type = TYPE;

		constexpr ptime() noexcept : m_time(0) {}

		~ptime() noexcept = default;

		constexpr ptime(const ptime &rhs) noexcept = default;
		constexpr ptime(ptime &&rhs) noexcept = default;
		constexpr explicit ptime(const internal_type &time) noexcept : m_time(time) {}
		constexpr explicit ptime(internal_type &&time) noexcept : m_time(time) {}
		C14CONSTEXPR ptime &operator=(const ptime &rhs) noexcept = default;
		C14CONSTEXPR ptime &operator=(ptime &&rhs) noexcept  = default;

		constexpr explicit ptime(const double t) = delete;
		//: m_time((internal_type) ( t * (double) resolution)) { }
		constexpr explicit ptime(const internal_type nom, const internal_type den) noexcept
		: m_time(nom * (RES / den)) { }

		C14CONSTEXPR ptime &operator+=(const ptime &rhs) noexcept { m_time += rhs.m_time; return *this; }
		C14CONSTEXPR ptime &operator-=(const ptime &rhs) noexcept { m_time -= rhs.m_time; return *this; }
		C14CONSTEXPR ptime &operator*=(const mult_type factor) noexcept { m_time *= static_cast<internal_type>(factor); return *this; }

		friend constexpr const ptime operator-(ptime lhs, const ptime rhs) noexcept
		{
			return ptime(lhs.m_time - rhs.m_time);
		}

		friend constexpr const ptime operator+(ptime lhs, const ptime rhs) noexcept
		{
			return ptime(lhs.m_time + rhs.m_time);
		}

		friend constexpr const ptime operator*(ptime lhs, const mult_type factor) noexcept
		{
			return ptime(lhs.m_time * factor);
		}

		friend constexpr mult_type operator/(const ptime lhs, const ptime rhs) noexcept
		{
			return static_cast<mult_type>(lhs.m_time / rhs.m_time);
		}

		friend constexpr bool operator<(const ptime lhs, const ptime rhs) noexcept
		{
			return (lhs.m_time < rhs.m_time);
		}

		friend constexpr bool operator>(const ptime lhs, const ptime rhs) noexcept
		{
			return (rhs < lhs);
		}

		friend constexpr bool operator<=(const ptime lhs, const ptime rhs) noexcept
		{
			return !(lhs > rhs);
		}

		friend constexpr bool operator>=(const ptime lhs, const ptime rhs) noexcept
		{
			return !(lhs < rhs);
		}

		friend constexpr bool operator==(const ptime lhs, const ptime rhs) noexcept
		{
			return lhs.m_time == rhs.m_time;
		}

		friend constexpr bool operator!=(const ptime lhs, const ptime rhs) noexcept
		{
			return !(lhs == rhs);
		}

		constexpr internal_type as_raw() const noexcept { return m_time; }

		template <typename FT, typename = std::enable_if<std::is_floating_point<FT>::value, FT>>
		constexpr FT
		as_fp() const noexcept
		{
			return static_cast<FT>(m_time) * inv_res<FT>();
		}

#if PUSE_FLOAT128
		constexpr __float128
		as_fp() const noexcept
		{
			return static_cast<__float128>(m_time) * inv_res<__float128>();
		}
#endif

		constexpr double as_double() const noexcept { return as_fp<double>(); }
		constexpr double as_float() const noexcept { return as_fp<float>(); }
		constexpr double as_long_double() const noexcept { return as_fp<long double>(); }

		// for save states ....
		C14CONSTEXPR internal_type *get_internaltype_ptr() noexcept { return &m_time; }

		static constexpr const ptime from_nsec(const internal_type ns) noexcept { return ptime(ns, UINT64_C(1000000000)); }
		static constexpr const ptime from_usec(const internal_type us) noexcept { return ptime(us, UINT64_C(   1000000)); }
		static constexpr const ptime from_msec(const internal_type ms) noexcept { return ptime(ms, UINT64_C(      1000)); }
		static constexpr const ptime from_sec(const internal_type s) noexcept   { return ptime(s,  UINT64_C(         1)); }
		static constexpr const ptime from_hz(const internal_type hz) noexcept { return ptime(1 , hz); }
		static constexpr const ptime from_raw(const internal_type raw) noexcept { return ptime(raw); }

		template <typename FT>
		static constexpr const typename std::enable_if<std::is_floating_point<FT>::value
#if PUSE_FLOAT128
			|| std::is_same<FT, __float128>::value
#endif
		, ptime>::type
		from_fp(const FT t) noexcept { return ptime(static_cast<internal_type>(plib::floor(t * static_cast<FT>(RES) + static_cast<FT>(0.5))), RES); }

		static constexpr const ptime from_double(const double t) noexcept
		{ return from_fp<double>(t); }

		static constexpr const ptime from_float(const float t) noexcept
		{ return from_fp<float>(t); }

		static constexpr const ptime from_long_double(const long double t) noexcept
		{ return from_fp<long double>(t); }

		static constexpr const ptime zero() noexcept { return ptime(0, RES); }
		static constexpr const ptime quantum() noexcept { return ptime(1, RES); }
		static constexpr const ptime never() noexcept { return ptime(plib::numeric_limits<internal_type>::max(), RES); }
		static constexpr const internal_type resolution() noexcept { return RES; }

		constexpr internal_type in_nsec() const noexcept { return m_time / (RES / UINT64_C(1000000000)); }
		constexpr internal_type in_usec() const noexcept { return m_time / (RES / UINT64_C(   1000000)); }
		constexpr internal_type in_msec() const noexcept { return m_time / (RES / UINT64_C(      1000)); }
		constexpr internal_type in_sec()  const noexcept { return m_time / (RES / UINT64_C(         1)); }

	private:
		template <typename FT>
		static constexpr FT inv_res() noexcept { return static_cast<FT>(1.0) / static_cast<FT>(RES); }
		internal_type m_time;
	};


} // namespace plib


#endif // PTIME_H_
