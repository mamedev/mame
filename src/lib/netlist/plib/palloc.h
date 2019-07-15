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
#include <type_traits>
#include <utility>
#include <vector>

#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
#include <malloc.h>
#endif

namespace plib {

	//============================================================
	// Standard arena_deleter
	//============================================================

	template <typename P, typename T>
	struct arena_deleter
	{
		//using arena_storage_type = P *;
		using arena_storage_type = typename std::conditional<P::is_stateless, P, P *>::type;
		template <typename X, typename Y = void>
		typename std::enable_if<!X::is_stateless, X&>::type getref(X *x) { return *x;}
		template <typename X, typename Y = void *>
		typename std::enable_if<std::remove_pointer<X>::type::is_stateless, X&>::type
		getref(X &x, Y y = nullptr)
		{
			unused_var(y);
			return x;
		}

		constexpr arena_deleter(arena_storage_type a = arena_storage_type()) noexcept
		: m_a(a) { }

		template<typename PU, typename U, typename = typename
			   std::enable_if<std::is_convertible< U*, T*>::value>::type>
		arena_deleter(const arena_deleter<PU, U> &rhs) noexcept : m_a(rhs.m_a) { }

		void operator()(T *p) //const
		{
			/* call destructor */
			p->~T();
			getref(m_a).deallocate(p);
		}
	//private:
		arena_storage_type m_a;
	};

	//============================================================
	// owned_ptr: smart pointer with ownership information
	//============================================================

	template <typename SC, typename D>
	class owned_ptr
	{
	public:

		using pointer = SC *;
		using element_type = SC;
		using deleter_type = D;

		owned_ptr()
		: m_ptr(nullptr), m_deleter(), m_is_owned(true) { }

		template <typename, typename>
		friend class owned_ptr;

		owned_ptr(pointer p, bool owned) noexcept
		: m_ptr(p), m_deleter(), m_is_owned(owned)
		{ }

		owned_ptr(pointer p, bool owned, D deleter) noexcept
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
		: m_ptr(r.m_ptr)
		, m_deleter(r.m_deleter)
		, m_is_owned(r.m_is_owned)
		{
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
			m_deleter = std::move(r.m_deleter);
			r.m_is_owned = false;
			r.m_ptr = nullptr;
			return *this;
		}

		template<typename DC, typename DC_D>
		owned_ptr(owned_ptr<DC, DC_D> &&r) noexcept
		: m_ptr(static_cast<pointer >(r.get()))
		, m_deleter(r.m_deleter)
		, m_is_owned(r.is_owned())
		{
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

		/**
		 * \brief Return @c true if the stored pointer is not null.
		 */
		explicit operator bool() const noexcept { return m_ptr != nullptr; }

		pointer  release()
		{
			pointer tmp = m_ptr;
			m_is_owned = false;
			m_ptr = nullptr;
			return tmp;
		}

		bool is_owned() const { return m_is_owned; }

		pointer  operator ->() const noexcept { return m_ptr; }
		typename std::add_lvalue_reference<element_type>::type operator *() const noexcept { return *m_ptr; }
		pointer  get() const noexcept { return m_ptr; }

		deleter_type& get_deleter() noexcept { return m_deleter; }
		const deleter_type& get_deleter() const noexcept { return m_deleter; }

	private:
		pointer m_ptr;
		D m_deleter;
		bool m_is_owned;
	};

	//============================================================
	// Arena allocator for use with containers
	//============================================================

	template <class ARENA, class T, std::size_t ALIGN = alignof(T)>
	class arena_allocator
	{
	public:
		using value_type = T;
		static constexpr const std::size_t align_size = ALIGN;
		using arena_type = ARENA;

		static_assert(align_size >= alignof(T) && (align_size % alignof(T)) == 0,
			"ALIGN must be greater than alignof(T) and a multiple");

		arena_allocator() noexcept
		: m_a(arena_type::instance())
		{ }

		~arena_allocator() noexcept = default;

		arena_allocator(const arena_allocator &rhs) noexcept = default;
		arena_allocator& operator=(const arena_allocator&) noexcept = delete;

		arena_allocator(arena_allocator&&) noexcept = default;
		arena_allocator& operator=(arena_allocator&&) = delete;

		arena_allocator(arena_type & a) noexcept : m_a(a)
		{
		}

		template <class U>
		arena_allocator(const arena_allocator<ARENA, U, ALIGN>& rhs) noexcept
		: m_a(rhs.m_a)
		{
		}

		template <class U> struct rebind
		{
			using other = arena_allocator<ARENA, U, ALIGN>;
		};

		T* allocate(std::size_t n)
		{
			return reinterpret_cast<T *>(m_a.allocate(ALIGN, sizeof(T) * n));
		}

		void deallocate(T* p, std::size_t n) noexcept
		{
			unused_var(n);
			m_a.deallocate(p);
		}

		template <class AR1, class T1, std::size_t A1, class AR2, class T2, std::size_t A2>
		friend bool operator==(const arena_allocator<AR1, T1, A1>& lhs,
			const arena_allocator<AR2, T2, A2>& rhs) noexcept;

		template <class AU, class U, std::size_t A> friend class arena_allocator;
	private:
		arena_type &m_a;
	};

	template <class AR1, class T1, std::size_t A1, class AR2, class T2, std::size_t A2>
	inline bool operator==(const arena_allocator<AR1, T1, A1>& lhs,
		const arena_allocator<AR2, T2, A2>& rhs) noexcept
	{
		return A1 == A2 && rhs.m_a == lhs.m_a;
	}
	template <class AR1, class T1, std::size_t A1, class AR2, class T2, std::size_t A2>
	inline bool operator!=(const arena_allocator<AR1, T1, A1>& lhs,
		const arena_allocator<AR2, T2, A2>& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	//============================================================
	//  Memory allocation
	//============================================================

	struct aligned_arena
	{
		static constexpr const bool is_stateless = true;
		template <class T, std::size_t ALIGN = alignof(T)>
		using allocator_type = arena_allocator<aligned_arena, T, ALIGN>;

		template <typename T>
		using owned_pool_ptr = plib::owned_ptr<T, arena_deleter<aligned_arena, T>>;

		static inline aligned_arena &instance()
		{
			static aligned_arena s_arena;
			return s_arena;
		}

		static inline void *allocate( size_t alignment, size_t size )
		{
			#if (USE_ALIGNED_ALLOCATION)
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
			#else
				unused_var(alignment);
				return ::operator new(size);
			#endif
		}

		static inline void deallocate( void *ptr )
		{
			#if (USE_ALIGNED_ALLOCATION)
				// NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
				free(ptr);
			#else
				::operator delete(ptr);
			#endif
		}

#if 0
		template<typename T, typename... Args>
		owned_pool_ptr<T> make_poolptr(Args&&... args)
		{
			auto *mem = allocate(alignof(T), sizeof(T));
			return owned_pool_ptr<T>(new (mem) T(std::forward<Args>(args)...), true, arena_deleter<aligned_arena, T>(*this));
		}
#endif
		template<typename T, typename... Args>
		owned_pool_ptr<T> make_poolptr(Args&&... args)
		{
			auto *mem = allocate(alignof(T), sizeof(T));
			try
			{
				auto *mema = new (mem) T(std::forward<Args>(args)...);
				return owned_pool_ptr<T>(mema, true, arena_deleter<aligned_arena, T>(*this));
			}
			catch (...)
			{
				deallocate(mem);
				throw;
			}
		}

	};

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

	// FIXME: remove
	template<typename T, typename... Args>
	inline T *pnew(Args&&... args)
	{
		auto *p = aligned_arena::allocate(alignof(T), sizeof(T));
		return new(p) T(std::forward<Args>(args)...);
	}

	template<typename T>
	inline void pdelete(T *ptr)
	{
		ptr->~T();
		aligned_arena::deallocate(ptr);
	}


	template <typename T>
	using unique_ptr = std::unique_ptr<T, arena_deleter<aligned_arena, T>>;

	template<typename T, typename... Args>
	plib::unique_ptr<T> make_unique(Args&&... args)
	{
		return plib::unique_ptr<T>(pnew<T>(std::forward<Args>(args)...));
	}

#if 0
	template<typename T, typename... Args>
	static owned_ptr<T> make_owned(Args&&... args)
	{
		return owned_ptr<T>(pnew<T>(std::forward<Args>(args)...), true);
	}
#endif


	template <class T, std::size_t ALIGN = alignof(T)>
	using aligned_allocator = aligned_arena::allocator_type<T, ALIGN>;

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
		static constexpr const std::size_t stride_size = lcm(align_size, value_size) / value_size;
	};

	template <typename T>
	struct align_traits<T, typename std::enable_if<has_align<T>::value, void>::type>
	{
		static constexpr const std::size_t align_size = T::align_size;
		static constexpr const std::size_t value_size = sizeof(typename T::value_type);
		static constexpr const std::size_t stride_size = lcm(align_size, value_size) / value_size;
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
