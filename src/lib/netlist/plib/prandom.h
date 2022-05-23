// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PRANDOM_H_
#define PRANDOM_H_

///
/// \file pmath.h
///

#include "pconfig.h"
#include "pgsl.h"
#include "pmath.h"
#include "ptypes.h"

//#include <algorithm>
//#include <cmath>
//#include <type_traits>
#include <array>

namespace plib
{

	/// \brief Mersenne Twister implementation which is state saveable
	///
	/// This is a Mersenne Twister implementation which is state saveable.
	/// It has been written following this wikipedia entry:
	///
	///     https://en.wikipedia.org/wiki/Mersenne_Twister
	///
	/// The implementation has basic support for the interface described here
	///
	///     https://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine
	///
	/// so that it can be used with the C++11 random environment
	///
	template<typename T,
		std::size_t w, std::size_t N, std::size_t m, std::size_t r,
		T a,
		std::size_t u, T d,
		std::size_t s, T b,
		std::size_t t, T c,
		std::size_t l, T f>
	class mersenne_twister_t
	{
	public:

		mersenne_twister_t()
		: m_p(N)
		{
			seed(5489);
		}

		static constexpr T min() noexcept { return T(0); }
		static constexpr T max() noexcept { return ~T(0) >> (sizeof(T)*8 - w); }

		template <typename ST>
		void save_state(ST &st)
		{
			st.save_item(m_p,  "index");
			st.save_item(m_mt, "mt");
		}

		void seed(T val) noexcept
		{
			const T lowest_w(~T(0) >> (sizeof(T)*8 - w));
			m_p = N;
			m_mt[0] = val;
			for (std::size_t i=1; i< N; i++)
				m_mt[i] = (f * (m_mt[i-1] ^ (m_mt[i-1] >> (w-2))) + i) & lowest_w;
		}

		T operator()() noexcept
		{
			const T lowest_w(~T(0) >> (sizeof(T)*8 - w));
			if (m_p >= N)
				twist();

			T y = m_mt[m_p++];
			y = y ^ ((y >> u) & d);
			y = y ^ ((y << s) & b);
			y = y ^ ((y << t) & c);
			y = y ^ (y >> l);

			return y & lowest_w;
		}

		void discard(std::size_t v) noexcept
		{
			if  (v > N - m_p)
			{
				v -= N - m_p;
				twist();
			}
			while (v > N)
			{
				v -= N;
				twist();
			}
			m_p += v;
		}

	private:
		void twist() noexcept
		{
			const T lowest_w(~T(0) >> (sizeof(T)*8 - w));
			const T lower_mask((T(1) << r) - 1); // That is, the binary number of r 1's
			const T upper_mask((~lower_mask) & lowest_w);

			for (std::size_t i=0; i<N; i++)
			{
				const T x((m_mt[i] & upper_mask) + (m_mt[(i+1) % N] & lower_mask));
				const T xA((x >> 1) ^ ((x & 1) ? a : 0));
				m_mt[i] = m_mt[(i + m) % N] ^ xA;
			 }
			m_p = 0;
		}

		std::size_t m_p;
		std::array<T, N> m_mt;
	};

	template <typename FT, typename T>
	FT normalize_uniform(T &p, FT m = constants<FT>::one(), FT b = constants<FT>::zero()) noexcept
	{
		constexpr const FT mmin(narrow_cast<FT>(T::min()));
		constexpr const FT mmax(narrow_cast<FT>(T::max()));
		// -> 0 to a
		return (narrow_cast<FT>(p())- mmin) / (mmax - mmin) * m - b;
	}

	template<typename FT>
	class uniform_distribution_t
	{
	public:
		uniform_distribution_t(FT dev)
		: m_stddev(dev) { }

		template <typename P>
		FT operator()(P &p) noexcept
		{
			// get -1 to 1
			return normalize_uniform(p, constants<FT>::two(), constants<FT>::one())
				* constants<FT>::sqrt3() * m_stddev;
		}

		template <typename ST>
		void save_state([[maybe_unused]] ST &st)
		{
			/* no state to save */
		}

	private:
		FT m_stddev;
	};

	template<typename FT>
	class normal_distribution_t
	{
	public:
		normal_distribution_t(FT dev)
		: m_p(m_buf.size()), m_stddev(dev) { }

		// Donald Knuth, Algorithm P (Polar method)

		template <typename P>
		FT operator()(P &p) noexcept
		{
			if (m_p >= m_buf.size())
				fill(p);
			return m_buf[m_p++];
		}

		template <typename ST>
		void save_state(ST &st)
		{
			st.save_item(m_p,   "m_p");
			st.save_item(m_buf, "m_buf");
		}

	private:

		template <typename P>
		void fill(P &p) noexcept
		{
			for (std::size_t i = 0; i < m_buf.size(); i += 2)
			{
				FT s;
				FT v1;
				FT v2;
				do
				{
					v1 = normalize_uniform(p, constants<FT>::two(), constants<FT>::one()); // [-1..1[
					v2 = normalize_uniform(p, constants<FT>::two(), constants<FT>::one()); // [-1..1[
					s = v1 * v1 + v2 * v2;
				} while (s >= constants<FT>::one());
				if (s == constants<FT>::zero())
				{
					m_buf[i] = s;
					m_buf[i+1] = s;
				}
				else
				{
					// last value without error for log(s)/s
					// double: 1.000000e-305
					// float: 9.999999e-37
					// FIXME: with 128 bit randoms log(s)/w will fail 1/(2^128) ~ 2.9e-39
					const FT m(m_stddev * plib::sqrt(-constants<FT>::two() * plib::log(s)/s));
					m_buf[i] = /*mean+*/ m * v1;
					m_buf[i+1] = /*mean+*/ m * v2;
				}
			}
			m_p = 0;
		}

		std::array<FT, 256> m_buf;
		std::size_t m_p;
		FT m_stddev;
	};

	using mt19937_64 = mersenne_twister_t<
		uint_fast64_t,
		64, 312, 156, 31,
		0xb5026f5aa96619e9ULL,
		29, 0x5555555555555555ULL,
		17, 0x71d67fffeda60000ULL,
		37, 0xfff7eee000000000ULL,
		43,
		6364136223846793005ULL>;

} // namespace plib

#endif // PRANDOM_H_
