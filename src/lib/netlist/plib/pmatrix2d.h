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

		constexpr T * operator[] (size_type row) noexcept
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

		// for compatibility with vrl variant
		void set(size_type r, size_type c, const T &v) noexcept
		{
			(*this)[r][c] = v;
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

	// variable row length matrix
	template<typename T, typename A = aligned_allocator<T>>
	class pmatrix2d_vrl
	{
	public:
		using size_type = std::size_t;
		using value_type = T;
		using allocator_type = A;

		static constexpr const size_type align_size = align_traits<A>::align_size;
		static constexpr const size_type stride_size = align_traits<A>::stride_size;
		pmatrix2d_vrl() noexcept
		: m_N(0), m_M(0), m_v()
		{
		}

		pmatrix2d_vrl(size_type N, size_type M)
		: m_N(N), m_M(M), m_v()
		{
			m_row.resize(N + 1, 0);
			m_v.resize(N); //FIXME
		}

		void resize(size_type N, size_type M)
		{
			m_N = N;
			m_M = M;
			m_row.resize(N + 1);
			for (std::size_t i = 0; i < m_N; i++)
				m_row[i] = 0;
			m_v.resize(N); //FIXME
		}

		constexpr T * operator[] (size_type row) noexcept
		{
			return &(m_v[m_row[row]]);
		}

		constexpr const T * operator[] (size_type row) const noexcept
		{
			return &(m_v[m_row[row]]);
		}

#if 0
		T & operator()(size_type r, size_type c) noexcept
		{
			return (*this)[r][c];
		}
#else
		void set(size_type r, size_type c, const T &v) noexcept
		{
			if (c + m_row[r] >= m_row[r + 1])
			{
				m_v.insert(m_v.begin() + m_row[r+1], v);
				for (size_type i = r + 1; i <= m_N; i++)
					m_row[i] = m_row[i] + 1;
			}
			else
				(*this)[r][c] = v;
		}
#endif
		//FIXME: no check!
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

		//FIXME: no check!
		size_type didx(size_type r, size_type c) const noexcept
		{
			return m_row[r] + c;
		}

		std::size_t tx() const { return m_v.size(); }
	private:

		size_type m_N;
		size_type m_M;
		std::vector<size_type, A> m_row;
		std::vector<T, A> m_v;
	};


} // namespace plib

#endif // PMATRIX2D_H_
