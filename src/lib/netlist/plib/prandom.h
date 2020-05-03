// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PRANDOM_H_
#define PRANDOM_H_

///
/// \file pmath.h
///

#include "pconfig.h"
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
	/// 	https://en.wikipedia.org/wiki/Mersenne_Twister
	///
	/// The implementation has basic support for the interface described here
	///
	/// 	https://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine
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
		: index(N+1)
		{
			seed(5489);
		}

		static constexpr T min() noexcept { return static_cast<T>(0); }
		static constexpr T max() noexcept { return ~T(0) >> (sizeof(T)*8 - w); }

		void seed(T val) noexcept
		{
			const T lowest_w(~T(0) >> (sizeof(T)*8 - w));
			index = N;
			mt[0] = val;
			for (std::size_t i=1; i< N; i++)
				mt[i] = (f * (mt[i-1] ^ (mt[i-1] >> (w-2))) + i) & lowest_w;
		}

		T operator()() noexcept
		{
			const T lowest_w(~T(0) >> (sizeof(T)*8 - w));
			if (index >= N)
				twist();

			T y = mt[index++];
			y = y ^ ((y >> u) & d);
			y = y ^ ((y << s) & b);
			y = y ^ ((y << t) & c);
			y = y ^ (y >> l);

			return y & lowest_w;
		}

		void discard(std::size_t v) noexcept
		{
			if  (v > N - index)
			{
				v -= N - index;
				twist();
			}
			while (v > N)
			{
				v -= N;
				twist();
			}
			index += v;
		}

	private:
		void twist()
		{
			const T lowest_w(~T(0) >> (sizeof(T)*8 - w));
			const T lower_mask((static_cast<T>(1) << r) - 1); // That is, the binary number of r 1's
			const T upper_mask((~lower_mask) & lowest_w);

			for (std::size_t i=0; i<N; i++)
			{
				const T x((mt[i] & upper_mask) + (mt[(i+1) % N] & lower_mask));
				const T xA((x >> 1) ^ ((x & 1) ? a : 0));
				mt[i] = mt[(i + m) % N] ^ xA;
			 }
			 index = 0;
		 }

		std::array<T, N> mt;
		std::size_t index;
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
