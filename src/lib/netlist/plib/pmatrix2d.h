// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pmatrix2d.h
 *
 * NxM regular matrix
 *
 */

#ifndef PMATRIX2D_H_
#define PMATRIX2D_H_

#include "palloc.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <type_traits>
#include <vector>

namespace plib
{


	template<typename T, typename A = aligned_allocator<T>>
	class pmatrix2d
	{
	public:
		using value_type = T;
		using allocator_type = A;

		static constexpr const std::size_t align_size = align_traits<A>::align_size;
		static constexpr const std::size_t stride_size = align_traits<A>::stride_size;
		pmatrix2d()
		: m_N(0), m_M(0), m_stride(8), m_v()
		{
		}

		pmatrix2d(std::size_t N, std::size_t M)
		: m_N(N), m_M(M), m_v()
		{
			m_stride = ((M + stride_size-1) / stride_size) * stride_size;
			m_v.resize(N * m_stride);
		}

		void resize(std::size_t N, std::size_t M)
		{
			m_N = N;
			m_M = M;
			m_stride = ((M + stride_size-1) / stride_size) * stride_size;
			m_v.resize(N * m_stride);
		}

		C14CONSTEXPR T * operator[] (std::size_t row) noexcept
		{
			return assume_aligned_ptr<T, align_size>(&m_v[m_stride * row]);
		}

		constexpr const T * operator[] (std::size_t row) const noexcept
		{
			return assume_aligned_ptr<T, align_size>(&m_v[m_stride * row]);
		}

		T & operator()(std::size_t r, std::size_t c) noexcept
		{
			return (*this)[r][c];
		}

		const T & operator()(std::size_t r, std::size_t c) const noexcept
		{
			return (*this)[r][c];
		}

	private:

		std::size_t m_N;
		std::size_t m_M;
		std::size_t m_stride;

		std::vector<T, A> m_v;
	};

} // namespace plib

#endif /* MAT_CR_H_ */
