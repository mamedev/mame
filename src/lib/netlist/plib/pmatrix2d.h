// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PMATRIX2D_H_
#define PMATRIX2D_H_

///
/// \file pmatrix2d.h
///

#include "palloc.h"

#include <algorithm>
#include <type_traits>
#include <vector>

namespace plib
{

	template<typename T, typename A = aligned_allocator<T>>
	class pmatrix2d
	{
	public:
		using size_type = std::size_t;
		using value_type = T;
		using allocator_type = A;

		static constexpr const size_type align_size = align_traits<A>::align_size;
		static constexpr const size_type stride_size = align_traits<A>::stride_size;
		pmatrix2d() noexcept
		: m_N(0), m_M(0), m_stride(8), m_v()
		{
		}

		pmatrix2d(size_type N, size_type M)
		: m_N(N), m_M(M), m_v()
		{
			m_stride = ((M + stride_size-1) / stride_size) * stride_size;
			m_v.resize(N * m_stride);
		}

		void resize(size_type N, size_type M)
		{
			m_N = N;
			m_M = M;
			m_stride = ((M + stride_size-1) / stride_size) * stride_size;
			m_v.resize(N * m_stride);
		}

		C14CONSTEXPR T * operator[] (size_type row) noexcept
		{
			return assume_aligned_ptr<T, align_size>(&m_v[m_stride * row]);
		}

		constexpr const T * operator[] (size_type row) const noexcept
		{
			return assume_aligned_ptr<T, align_size>(&m_v[m_stride * row]);
		}

		T & operator()(size_type r, size_type c) noexcept
		{
			return (*this)[r][c];
		}

		const T & operator()(size_type r, size_type c) const noexcept
		{
			return (*this)[r][c];
		}

		T * data() noexcept
		{
			return m_v.data();
		}

		const T * data() const noexcept
		{
			return m_v.data();
		}

		size_type didx(size_type r, size_type c) const noexcept
		{
			return m_stride * r + c;
		}
	private:

		size_type m_N;
		size_type m_M;
		size_type m_stride;

		std::vector<T, A> m_v;
	};

} // namespace plib

#endif // PMATRIX2D_H_
