// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * parray.h
 *
 */

#ifndef PARRAY_H_
#define PARRAY_H_

#include "pconfig.h"
#include "pexception.h"

#include <memory>
#include <utility>
#include <vector>
#include <array>
#include <type_traits>

namespace plib {

	template <typename FT, int SIZE>
	struct sizeabs
	{
		static constexpr std::size_t ABS() { return (SIZE < 0) ? static_cast<std::size_t>(0 - SIZE) : static_cast<std::size_t>(SIZE); }
		typedef typename std::array<FT, ABS()> container;
	};

	template <typename FT>
	struct sizeabs<FT, 0>
	{
		static constexpr const std::size_t ABS = 0;
		typedef typename std::vector<FT> container;
	};

	/**
	 * \brief Array with preallocated or dynamic allocation
	 *
	 * Passing SIZE > 0 has the same functionality as a std::array.
	 * SIZE = 0 is pure dynamic allocation, the actual array size is passed to the
	 * constructor.
	 * SIZE < 0 reserves std::abs(SIZE) elements statically in place allocated. The
	 * actual size is passed in by the constructor.
	 * This array is purely intended for HPC application where depending on the
	 * architecture a preference dynamic/static has to be made.
	 *
	 * This struct is not intended to be a full replacement to std::array.
	 * It is a subset to enable switching between dynamic and static allocation.
	 * I consider > 10% performance difference to be a use case.
	 */

	template <typename FT, int SIZE>
	struct parray
	{
	public:
	    static constexpr std::size_t SIZEABS() { return sizeabs<FT, SIZE>::ABS(); }

		typedef typename sizeabs<FT, SIZE>::container base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::reference reference;
		typedef typename base_type::const_reference const_reference;

		template <int X = SIZE >
		parray(size_type size, typename std::enable_if<X==0, int>::type = 0)
		: m_a(size), m_size(size)
		{
		}

#if 0
		/* allow construction in fixed size arrays */
		template <int X = SIZE >
		parray(typename std::enable_if<X==0, int>::type = 0)
		: m_size(0)
		{
		}
#endif
		template <int X = SIZE >
		parray(size_type size, typename std::enable_if<X!=0, int>::type = 0)
		: m_size(size)
		{
			if (SIZE < 0 && size > SIZEABS())
				throw plib::pexception("parray: size error " + plib::to_string(size) + ">" + plib::to_string(SIZEABS()));
			else if (SIZE > 0 && size != SIZEABS())
				throw plib::pexception("parray: size error");
		}

	    inline size_type size() const noexcept { return SIZE <= 0 ? m_size : SIZEABS(); }

	    constexpr size_type max_size() const noexcept { return base_type::max_size(); }

	    bool empty() const noexcept { return size() == 0; }

#if 0
	    reference operator[](size_type i) /*noexcept*/
	    {
	    	if (i >= m_size) throw plib::pexception("limits error " + to_string(i) + ">=" + to_string(m_size));
	    	return m_a[i];
	    }
	    const_reference operator[](size_type i) const /*noexcept*/
	    {
	    	if (i >= m_size) throw plib::pexception("limits error " + to_string(i) + ">=" + to_string(m_size));
	    	return m_a[i];
	    }
#else
	    reference operator[](size_type i) noexcept { return m_a[i]; }
	    constexpr const_reference operator[](size_type i) const noexcept { return m_a[i]; }
#endif
	    FT * data() noexcept { return m_a.data(); }
	    const FT * data() const noexcept { return m_a.data(); }

	private:
		base_type m_a;
		size_type m_size;
	};
}

#endif /* PARRAY_H_ */
