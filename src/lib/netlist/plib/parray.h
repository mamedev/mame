// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PARRAY_H_
#define PARRAY_H_

///
/// \file parray.h
///

#include "palloc.h"
#include "pconfig.h"
#include "pexception.h"
#include "pstrutil.h"

#include <array>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace plib {

	template <typename FT, int SIZE>
	struct sizeabs
	{
		static constexpr std::size_t ABS() { return (SIZE < 0) ? static_cast<std::size_t>(0 - SIZE) : static_cast<std::size_t>(SIZE); }
		using container = typename std::array<FT, ABS()> ;
	};

	template <typename FT>
	struct sizeabs<FT, 0>
	{
		static constexpr std::size_t ABS() { return 0; }
		//using container = typename std::vector<FT, arena_allocator<mempool, FT, 64>>;
		using container = typename std::vector<FT, aligned_allocator<FT, PALIGN_VECTOROPT>>;
	};

	/// \brief Array with preallocated or dynamic allocation.
	///
	/// Passing SIZE > 0 has the same functionality as a std::array.
	/// SIZE = 0 is pure dynamic allocation, the actual array size is passed to the
	/// constructor.
	/// SIZE < 0 reserves abs(SIZE) elements statically in place allocated. The
	/// actual size is passed in by the constructor.
	/// This array is purely intended for HPC application where depending on the
	/// architecture a preference dynamic/static has to be made.
	///
	/// This struct is not intended to be a full replacement to std::array.
	/// It is a subset to enable switching between dynamic and static allocation.
	/// I consider > 10% performance difference to be a use case.
	///

	template <typename FT, int SIZE>
	struct parray
	{
	public:
		static constexpr std::size_t SIZEABS() { return sizeabs<FT, SIZE>::ABS(); }

		using base_type = typename sizeabs<FT, SIZE>::container;
		using size_type = typename base_type::size_type;
		using reference = typename base_type::reference;
		using const_reference = typename base_type::const_reference;
		using value_type = typename base_type::value_type;

		template <int X = SIZE >
		parray(size_type size, typename std::enable_if<(X==0), int>::type = 0)
		: m_a(size), m_size(size)
		{
		}

		template <int X = SIZE >
		parray(size_type size, FT val, typename std::enable_if<(X==0), int>::type = 0)
		: m_a(size, val), m_size(size)
		{
		}

		template <int X = SIZE >
		parray(size_type size, typename std::enable_if<(X != 0), int>::type = 0)
		: m_size(size)
		{
			if ((SIZE < 0 && size > SIZEABS())
				|| (SIZE > 0 && size != SIZEABS()))
				pthrow<pexception>("parray: size error " + plib::to_string(size) + ">" + plib::to_string(SIZE));
		}

		template <int X = SIZE >
		parray(size_type size, FT val, typename std::enable_if<(X != 0), int>::type = 0)
		: m_size(size)
		{
			if ((SIZE < 0 && size > SIZEABS())
				|| (SIZE > 0 && size != SIZEABS()))
				pthrow<plib::pexception>("parray: size error " + plib::to_string(size) + ">" + plib::to_string(SIZE));
			m_a.fill(val);
		}


		// allow construction in fixed size arrays
		parray()
		: m_size(SIZEABS())
		{
		}

		// osx clang doesn't like COPYASSIGNMOVE(parray, default)
		// it will generate some weird error messages about move assignment
		// constructor having a different noexcept status.

		parray(const parray &rhs) : m_a(rhs.m_a), m_size(rhs.m_size) {}
		parray(parray &&rhs) noexcept : m_a(std::move(rhs.m_a)), m_size(std::move(rhs.m_size)) {}
		parray &operator=(const parray &rhs) noexcept
		{
			if (this != &rhs)
			{
				m_a = rhs.m_a;
				m_size = rhs.m_size;
			}
			return *this;
		}

		parray &operator=(parray &&rhs) noexcept { std::swap(m_a,rhs.m_a); std::swap(m_size, rhs.m_size); return *this; }

		~parray() noexcept = default;

		base_type &as_base() noexcept { return m_a; }

		inline size_type size() const noexcept { return SIZE <= 0 ? m_size : SIZEABS(); }

		constexpr size_type max_size() const noexcept { return base_type::max_size(); }

		bool empty() const noexcept { return size() == 0; }

		C14CONSTEXPR reference operator[](size_type i) noexcept
		{
			return assume_aligned_ptr<FT, PALIGN_VECTOROPT>(&m_a[0])[i];
		}
		constexpr const_reference operator[](size_type i) const noexcept
		{
			return assume_aligned_ptr<FT, PALIGN_VECTOROPT>(&m_a[0])[i];
		}

		FT * data() noexcept { return assume_aligned_ptr<FT, PALIGN_VECTOROPT>(m_a.data()); }
		const FT * data() const noexcept { return assume_aligned_ptr<FT, PALIGN_VECTOROPT>(m_a.data()); }

	private:
		PALIGNAS_VECTOROPT()
		base_type               m_a;
		PALIGNAS_CACHELINE()
		size_type               m_size;
	};

	template <typename FT, int SIZE1, int SIZE2>
	struct parray2D : public parray<parray<FT, SIZE2>, SIZE1>
	{
	public:

		using size_type = std::size_t;

		parray2D(size_type size1, size_type size2)
		: parray<parray<FT, SIZE2>, SIZE1>(size1)
		{
			if (SIZE2 <= 0)
			{
				for (size_type i=0; i < this->size(); i++)
					(*this)[i] = parray<FT, SIZE2>(size2);
			}
		}

		COPYASSIGNMOVE(parray2D, default)

		~parray2D() noexcept = default;

	};

} // namespace plib



#endif // PARRAY_H_
