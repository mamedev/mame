// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * palloc.h
 *
 */

#ifndef PALLOC_H_
#define PALLOC_H_

#include "pconfig.h"
#include "pstring.h"
#include "ptypes.h"

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <utility>
#include <vector>

#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
#include <malloc.h>
#endif

namespace plib {

	//============================================================
	//  Memory allocation
	//============================================================

#if (USE_ALIGNED_ALLOCATION)
	static inline void *paligned_alloc( size_t alignment, size_t size )
	{
#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
		return _aligned_malloc(size, alignment);
#elif defined(__APPLE__)
		void* p;
		if (::posix_memalign(&p, alignment, size) != 0) {
			p = nullptr;
		}
		return p;
#else
		return aligned_alloc(alignment, size);
#endif
	}

	static inline void pfree( void *ptr )
	{
		// NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
		free(ptr);
	}

#else
	static inline void *paligned_alloc( size_t alignment, size_t size )
	{
		unused_var(alignment);
		return ::operator new(size);
	}

	static inline void pfree( void *ptr )
	{
		::operator delete(ptr);
	}
#endif

	template <typename T, std::size_t ALIGN>
	/*inline */ C14CONSTEXPR T *assume_aligned_ptr(T *p) noexcept
	{
		static_assert(ALIGN >= alignof(T), "Alignment must be greater or equal to alignof(T)");
		static_assert(is_pow2(ALIGN), "Alignment must be a power of 2");
		//auto t = reinterpret_cast<std::uintptr_t>(p);
		//if (t & (ALIGN-1))
		//  printf("alignment error!");
#if (USE_ALIGNED_HINTS)
		return reinterpret_cast<T *>(__builtin_assume_aligned(p, ALIGN));
#else
		return p;
#endif
	}

	template <typename T, std::size_t ALIGN>
	constexpr const T *assume_aligned_ptr(const T *p) noexcept
	{
		static_assert(ALIGN >= alignof(T), "Alignment must be greater or equal to alignof(T)");
		static_assert(is_pow2(ALIGN), "Alignment must be a power of 2");
#if (USE_ALIGNED_HINTS)
		return reinterpret_cast<const T *>(__builtin_assume_aligned(p, ALIGN));
#else
		return p;
#endif
	}

	template<typename T, typename... Args>
	inline T *pnew(Args&&... args)
	{
		auto *p = paligned_alloc(alignof(T), sizeof(T));
		return new(p) T(std::forward<Args>(args)...);
	}

	template<typename T>
	inline void pdelete(T *ptr)
	{
		ptr->~T();
		pfree(ptr);
	}

	template<typename T>
	inline T* pnew_array(const std::size_t num)
	{
		return new T[num]();
	}

	template<typename T>
	inline void pdelete_array(T *ptr)
	{
		delete [] ptr;
	}

	template <typename T>
	struct pdefault_deleter
	{
		constexpr pdefault_deleter() noexcept = default;

		template<typename U, typename = typename
			   std::enable_if<std::is_convertible< U*, T*>::value>::type>
		pdefault_deleter(const pdefault_deleter<U>&) noexcept { }

		void operator()(T *p) const
		{
			pdelete(p);
		}
	};

	template <typename SC, typename D = pdefault_deleter<SC>>
	class owned_ptr
	{
	public:
		owned_ptr()
		: m_ptr(nullptr), m_is_owned(true) { }

		template <typename, typename>
		friend class owned_ptr;

		owned_ptr(SC *p, bool owned) noexcept
		: m_ptr(p), m_deleter(), m_is_owned(owned)
		{ }

		owned_ptr(SC *p, bool owned, D deleter) noexcept
		: m_ptr(p), m_deleter(deleter), m_is_owned(owned)
		{ }


		owned_ptr(const owned_ptr &r) = delete;
		owned_ptr & operator =(owned_ptr &r) = delete;

		template<typename DC, typename DC_D>
		owned_ptr & operator =(owned_ptr<DC, DC_D> &&r)
		{
			if (m_is_owned && (m_ptr != nullptr))
				//delete m_ptr;
				m_deleter(m_ptr);
			m_is_owned = r.m_is_owned;
			m_ptr = r.m_ptr;
			m_deleter = r.m_deleter;
			r.m_is_owned = false;
			r.m_ptr = nullptr;
			return *this;
		}

		owned_ptr(owned_ptr &&r) noexcept
		{
			m_is_owned = r.m_is_owned;
			m_ptr = r.m_ptr;
			m_deleter = r.m_deleter;
			r.m_is_owned = false;
			r.m_ptr = nullptr;
		}

		owned_ptr &operator=(owned_ptr &&r) noexcept
		{
			if (m_is_owned && (m_ptr != nullptr))
				//delete m_ptr;
				m_deleter(m_ptr);
			m_is_owned = r.m_is_owned;
			m_ptr = r.m_ptr;
			m_deleter = r.m_deleter;
			r.m_is_owned = false;
			r.m_ptr = nullptr;
			return *this;
		}

		template<typename DC, typename DC_D>
		owned_ptr(owned_ptr<DC, DC_D> &&r) noexcept
		{
			m_ptr = static_cast<SC *>(r.get());
			m_is_owned = r.is_owned();
			m_deleter = r.m_deleter;
			r.release();
		}

		~owned_ptr() noexcept
		{
			if (m_is_owned && (m_ptr != nullptr))
			{
				//delete m_ptr;
				m_deleter(m_ptr);
			}
			m_is_owned = false;
			m_ptr = nullptr;
		}
		SC * release()
		{
			SC *tmp = m_ptr;
			m_is_owned = false;
			m_ptr = nullptr;
			return tmp;
		}

		bool is_owned() const { return m_is_owned; }

		SC * operator ->() const { return m_ptr; }
		SC & operator *() const { return *m_ptr; }
		SC * get() const { return m_ptr; }
	private:
		SC *m_ptr;
		D m_deleter;
		bool m_is_owned;
	};

	template <typename T>
	using unique_ptr = std::unique_ptr<T, pdefault_deleter<T>>;

	template<typename T, typename... Args>
	plib::unique_ptr<T> make_unique(Args&&... args)
	{
		return plib::unique_ptr<T>(pnew<T>(std::forward<Args>(args)...));
	}

	template<typename T, typename... Args>
	static owned_ptr<T> make_owned(Args&&... args)
	{
		owned_ptr<T> a(pnew<T>(std::forward<Args>(args)...), true);
		return std::move(a);
	}

	//============================================================
	// Aligned allocator for use with containers
	//============================================================

	template <class T, std::size_t ALIGN = alignof(T)>
	class aligned_allocator
	{
	public:
		using value_type = T;
		static constexpr const std::size_t align_size = ALIGN;

		static_assert(align_size >= alignof(T) && (align_size % alignof(T)) == 0,
			"ALIGN must be greater than alignof(T) and a multiple");

		aligned_allocator() noexcept = default;
		~aligned_allocator() noexcept = default;

		aligned_allocator(const aligned_allocator&) noexcept = default;
		aligned_allocator& operator=(const aligned_allocator&) noexcept = delete;

		aligned_allocator(aligned_allocator&&) noexcept = default;
		aligned_allocator& operator=(aligned_allocator&&) = delete;

		template <class U>
		aligned_allocator(const aligned_allocator<U, ALIGN>& rhs) noexcept
		{
			unused_var(rhs);
		}

		template <class U> struct rebind
		{
			using other = aligned_allocator<U, ALIGN>;
		};

		T* allocate(std::size_t n)
		{
			return reinterpret_cast<T *>(paligned_alloc(ALIGN, sizeof(T) * n));
		}

		void deallocate(T* p, std::size_t n) noexcept
		{
			unused_var(n);
			pfree(p);
		}

		template <class T1, std::size_t A1, class U, std::size_t A2>
		friend bool operator==(const aligned_allocator<T1, A1>& lhs,
			const aligned_allocator<U, A2>& rhs) noexcept;

		template <class U, std::size_t A> friend class aligned_allocator;
	};

	template <class T1, std::size_t A1, class U, std::size_t A2>
	/*friend*/ inline bool operator==(const aligned_allocator<T1, A1>& lhs,
		const aligned_allocator<U, A2>& rhs) noexcept
	{
		unused_var(lhs, rhs);
		return A1 == A2;
	}
	template <class T1, std::size_t A1, class U, std::size_t A2>
	/*friend*/ inline bool operator!=(const aligned_allocator<T1, A1>& lhs,
		const aligned_allocator<U, A2>& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	//============================================================
	// traits to determine alignment size and stride size
	// from types supporting alignment
	//============================================================

	PDEFINE_HAS_MEMBER(has_align, align_size);

	template <typename T, typename X = void>
	struct align_traits
	{
		static constexpr const std::size_t align_size = alignof(std::max_align_t);
		static constexpr const std::size_t value_size = sizeof(typename T::value_type);
#if 0
		static constexpr const std::size_t stride_size =
			((value_size % align_size) == 0 ? 1 //T is a multiple of align_size
			 : ((align_size % value_size) != 0 ? align_size   // align_size is not a multiple of T
			 : align_size / value_size));
#else
		static constexpr const std::size_t stride_size = lcm(align_size, value_size) / value_size;
#endif
	};

	template <typename T>
	struct align_traits<T, typename std::enable_if<has_align<T>::value, void>::type>
	{
		static constexpr const std::size_t align_size = T::align_size;
		static constexpr const std::size_t value_size = sizeof(typename T::value_type);
#if 0
		static constexpr const std::size_t stride_size =
			((value_size % align_size) == 0 ? 1 //T is a multiple of align_size
			 : ((align_size % value_size) != 0 ? align_size   // align_size is not a multiple of T
			 : align_size / value_size));
#else
		static constexpr const std::size_t stride_size = lcm(align_size, value_size) / value_size;
#endif
	};

	//============================================================
	// Aligned vector
	//============================================================

	// FIXME: needs a separate file
	template <class T, std::size_t ALIGN = alignof(T)>
	class aligned_vector : public std::vector<T, aligned_allocator<T, ALIGN>>
	{
	public:
		using base = std::vector<T, aligned_allocator<T, ALIGN>>;

		using reference = typename base::reference;
		using const_reference = typename base::const_reference;
		using pointer = typename base::pointer;
		using const_pointer = typename base::const_pointer;
		using size_type = typename base::size_type;

		using base::base;

		C14CONSTEXPR reference operator[](size_type i) noexcept
		{
			return assume_aligned_ptr<T, ALIGN>(&(base::operator[](0)))[i];
		}
		constexpr const_reference operator[](size_type i) const noexcept
		{
			return assume_aligned_ptr<T, ALIGN>(&(base::operator[](0)))[i];
		}

		pointer data() noexcept { return assume_aligned_ptr<T, ALIGN>(base::data()); }
		const_pointer data() const noexcept { return assume_aligned_ptr<T, ALIGN>(base::data()); }

	};

} // namespace plib

#endif /* PALLOC_H_ */
