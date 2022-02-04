// license:BSD-3-Clause
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

	template<typename T, typename A = aligned_arena>
	class pmatrix2d
	{
	public:
		using size_type = std::size_t;
		using arena_type = A;
		using allocator_type = typename A::template allocator_type<T, PALIGN_VECTOROPT>;

		static constexpr const size_type align_size = align_traits<allocator_type>::align_size;
		static constexpr const size_type stride_size = align_traits<allocator_type>::stride_size;

		using value_type = T;
		using reference = T &;
		using const_reference = const T &;

		using pointer = T *;
		using const_pointer = const T *;

		pmatrix2d() noexcept
		: m_N(0), m_M(0), m_stride(8), m_v(nullptr)
		{
		}

		pmatrix2d(size_type N, size_type M)
		: m_N(N), m_M(M), m_v()
		{
			gsl_Expects(N>0);
			gsl_Expects(M>0);
			m_stride = ((M + stride_size-1) / stride_size) * stride_size;
			m_v = m_a.allocate(N * m_stride);
			for (std::size_t i = 0; i < N * m_stride; i++)
				::new(&m_v[i]) T();
		}

		pmatrix2d(const pmatrix2d &) = delete;
		pmatrix2d &operator=(const pmatrix2d &) = delete;
		pmatrix2d(pmatrix2d &&) = delete;
		pmatrix2d &operator=(pmatrix2d &&) = delete;

		~pmatrix2d()
		{
			if (m_v != nullptr)
			{
				for (std::size_t i = 0; i < m_N * m_stride; i++)
					(&m_v[i])->~T();
				m_a.deallocate(m_v, m_N * m_stride);
			}
		}

		void resize(size_type N, size_type M)
		{
			gsl_Expects(N>0);
			gsl_Expects(M>0);
			if (m_v != nullptr)
			{
				for (std::size_t i = 0; i < N * m_stride; i++)
					(&m_v[i])->~T();
				m_a.deallocate(m_v, N * m_stride);
			}
			m_N = N;
			m_M = M;
			m_stride = ((M + stride_size-1) / stride_size) * stride_size;
			m_v = m_a.allocate(N * m_stride);
			for (std::size_t i = 0; i < N * m_stride; i++)
				::new(&m_v[i]) T();
		}

		constexpr pointer operator[] (size_type row) noexcept
		{
			return &m_v[m_stride * row];
		}

		constexpr const_pointer operator[] (size_type row) const noexcept
		{
			return &m_v[m_stride * row];
		}

		reference operator()(size_type r, size_type c) noexcept
		{
			return (*this)[r][c];
		}

		const_reference operator()(size_type r, size_type c) const noexcept
		{
			return (*this)[r][c];
		}

		// for compatibility with vrl variant
		void set(size_type r, size_type c, const value_type &v) noexcept
		{
			(*this)[r][c] = v;
		}

		pointer data() noexcept
		{
			return m_v;
		}

		const_pointer data() const noexcept
		{
			return m_v;
		}

		size_type didx(size_type r, size_type c) const noexcept
		{
			return m_stride * r + c;
		}
	private:

		size_type m_N;
		size_type m_M;
		size_type m_stride;

		T * __restrict m_v;

		allocator_type m_a;
	};

	// variable row length matrix
	template<typename T, typename A = aligned_arena>
	class pmatrix2d_vrl
	{
	public:
		using size_type = std::size_t;
		using value_type = T;
		using arena_type = A;
		using allocator_type = typename A::template allocator_type<T, PALIGN_VECTOROPT>;

		static constexpr const size_type align_size = align_traits<allocator_type>::align_size;
		static constexpr const size_type stride_size = align_traits<allocator_type>::stride_size;

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

		pmatrix2d_vrl(const pmatrix2d_vrl &) = default;
		pmatrix2d_vrl &operator=(const pmatrix2d_vrl &) = default;
		pmatrix2d_vrl(pmatrix2d_vrl &&) = default;
		pmatrix2d_vrl &operator=(pmatrix2d_vrl &&) = default;

		~pmatrix2d_vrl() = default;

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

		//FIXME: no check!
		T & operator()(size_type r, size_type c) noexcept
		{
			return (*this)[r][c];
		}

		void set(size_type r, size_type c, const T &v) noexcept
		{
			if (c + m_row[r] >= m_row[r + 1])
			{
				m_v.insert(m_v.begin() + narrow_cast<std::ptrdiff_t>(m_row[r+1]), v);
				for (size_type i = r + 1; i <= m_N; i++)
					m_row[i] = m_row[i] + 1;
			}
			else
				(*this)[r][c] = v;
		}

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

		size_type colcount(size_type row) const noexcept
		{
			return m_row[row + 1] - m_row[row];
		}

		size_type tx() const { return m_v.size(); }
	private:

		size_type m_N;
		size_type m_M;
		std::vector<size_type, typename A::template allocator_type<size_type>> m_row;
		std::vector<T, allocator_type> m_v;
	};


} // namespace plib

#endif // PMATRIX2D_H_
