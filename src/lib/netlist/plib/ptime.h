// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PTIME_H_
#define PTIME_H_

///
/// \file ptime.h
///

#include <type_traits>

#include "pconfig.h"
#include "pmath.h" // std::floor
#include "ptypes.h"

// ----------------------------------------------------------------------------------------
// netlist_time
// ----------------------------------------------------------------------------------------

namespace plib
{

	template <typename T, typename U>
	struct ptime_le
	{
		const static bool value = sizeof(T) <= sizeof(U);
	};

#if 0
	template<typename T, typename U>
	struct ptime_res {
		using type = typename std::conditional<sizeof(T) >= sizeof(U), T, U>::type;
	};
#endif

	template <typename TYPE, TYPE RES>
	struct ptime final
	{
	public:

		using internal_type = TYPE;
		using mult_type = TYPE;

		template <typename altTYPE, altTYPE altRES>
		friend struct ptime;

		constexpr ptime() noexcept : m_time(0) {}

		~ptime() noexcept = default;

		constexpr ptime(const ptime &rhs) noexcept = default;
		constexpr ptime(ptime &&rhs) noexcept = default;
		constexpr explicit ptime(const internal_type &time) noexcept : m_time(time) {}
		constexpr explicit ptime(internal_type &&time) noexcept : m_time(time) {}
		C14CONSTEXPR ptime &operator=(const ptime &rhs) noexcept = default;
		C14CONSTEXPR ptime &operator=(ptime &&rhs) noexcept  = default;

		constexpr explicit ptime(const double t) = delete;
		constexpr explicit ptime(const internal_type nom, const internal_type den) noexcept
		: m_time(nom * (RES / den)) { }

		template <typename O>
		constexpr explicit ptime(const ptime<O, RES> &rhs) noexcept
		: m_time(static_cast<TYPE>(rhs.m_time))
		{
		}

		template <typename O>
		C14CONSTEXPR ptime &operator+=(const ptime<O, RES> &rhs) noexcept
		{
			static_assert(ptime_le<ptime<O, RES>, ptime>::value, "Invalid ptime type");
			m_time += rhs.m_time;
			return *this;
		}
		template <typename O>
		C14CONSTEXPR ptime &operator-=(const ptime<O, RES> &rhs) noexcept
		{
			static_assert(ptime_le<ptime<O, RES>, ptime>::value, "Invalid ptime type");
			m_time -= rhs.m_time;
			return *this;
		}

		template <typename M>
		C14CONSTEXPR ptime &operator*=(const M &factor) noexcept
		{
			static_assert(plib::is_integral<M>::value, "Factor must be an integral type");
			m_time *= factor;
			return *this;
		}

		template <typename O>
		constexpr ptime operator-(const ptime<O, RES> &rhs) const noexcept
		{
			static_assert(ptime_le<ptime<O, RES>, ptime>::value, "Invalid ptime type");
			return ptime(m_time - rhs.m_time);
		}

		template <typename O>
		constexpr ptime operator+(const ptime<O, RES> &rhs) const noexcept
		{
			static_assert(ptime_le<ptime<O, RES>, ptime>::value, "Invalid ptime type");
			return ptime(m_time + rhs.m_time);
		}

		template <typename M>
		constexpr ptime operator*(const M &factor) const noexcept
		{
			static_assert(plib::is_integral<M>::value, "Factor must be an integral type");
			return ptime(m_time * static_cast<mult_type>(factor));
		}

		template <typename O>
		constexpr mult_type operator/(const ptime<O, RES> &rhs) const noexcept
		{
			static_assert(ptime_le<ptime<O, RES>, ptime>::value, "Invalid ptime type");
			return static_cast<mult_type>(m_time / rhs.m_time);
		}

		friend constexpr bool operator<(const ptime &lhs, const ptime &rhs) noexcept
		{
			return (lhs.m_time < rhs.m_time);
		}

		friend constexpr bool operator>(const ptime &lhs, const ptime &rhs) noexcept
		{
			return (rhs < lhs);
		}

		friend constexpr bool operator<=(const ptime &lhs, const ptime &rhs) noexcept
		{
			return !(lhs > rhs);
		}

		friend constexpr bool operator>=(const ptime &lhs, const ptime &rhs) noexcept
		{
			return !(lhs < rhs);
		}

		friend constexpr bool operator==(const ptime &lhs, const ptime &rhs) noexcept
		{
			return lhs.m_time == rhs.m_time;
		}

		friend constexpr bool operator!=(const ptime &lhs, const ptime &rhs) noexcept
		{
			return !(lhs == rhs);
		}

		constexpr internal_type as_raw() const noexcept { return m_time; }

		template <typename FT, typename = std::enable_if<plib::is_floating_point<FT>::value, FT>>
		constexpr FT
		as_fp() const noexcept
		{
			return static_cast<FT>(m_time) * inv_res<FT>();
		}

		constexpr double as_double() const noexcept { return as_fp<double>(); }
		constexpr double as_float() const noexcept { return as_fp<float>(); }
		constexpr double as_long_double() const noexcept { return as_fp<long double>(); }


		constexpr ptime shl(unsigned shift) const noexcept { return ptime(m_time << shift); }
		constexpr ptime shr(unsigned shift) const noexcept { return ptime(m_time >> shift); }

		// for save states ....
		C14CONSTEXPR internal_type *get_internaltype_ptr() noexcept { return &m_time; }

		static constexpr ptime from_nsec(internal_type ns) noexcept { return ptime(ns, UINT64_C(1000000000)); }
		static constexpr ptime from_usec(internal_type us) noexcept { return ptime(us, UINT64_C(   1000000)); }
		static constexpr ptime from_msec(internal_type ms) noexcept { return ptime(ms, UINT64_C(      1000)); }
		static constexpr ptime from_sec(internal_type s) noexcept   { return ptime(s,  UINT64_C(         1)); }
		static constexpr ptime from_hz(internal_type hz) noexcept { return ptime(1 , hz); }
		static constexpr ptime from_raw(internal_type raw) noexcept { return ptime(raw); }

		template <typename FT>
		static constexpr typename std::enable_if<plib::is_floating_point<FT>::value, ptime>::type
		from_fp(FT t) noexcept { return ptime(static_cast<internal_type>(plib::floor(t * static_cast<FT>(RES) + static_cast<FT>(0.5))), RES); }

		static constexpr ptime from_double(double t) noexcept
		{ return from_fp<double>(t); }

		static constexpr ptime from_float(float t) noexcept
		{ return from_fp<float>(t); }

		static constexpr ptime from_long_double(long double t) noexcept
		{ return from_fp<long double>(t); }

		static constexpr ptime zero() noexcept { return ptime(0, RES); }
		static constexpr ptime quantum() noexcept { return ptime(1, RES); }
		static constexpr ptime never() noexcept { return ptime(plib::numeric_limits<internal_type>::max(), RES); }
		static constexpr internal_type resolution() noexcept { return RES; }

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
