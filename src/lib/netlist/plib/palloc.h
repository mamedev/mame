// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PALLOC_H_
#define PALLOC_H_

///
/// \file palloc.h
///

#include "pconfig.h"
#include "pgsl.h"
#include "pgsl.h"
#include "pmath.h"  // FIXME: only uses lcm ... move to ptypes.
#include "ptypes.h"

#include <algorithm>
#include <cstddef>      // for std::max_align_t (usually long long)
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
#include <malloc.h>
#endif

namespace plib {

	//============================================================
	// aligned types
	//============================================================

#if 0
#if (PUSE_ALIGNED_HINTS)
	template <typename T, std::size_t A>
	using aligned_type __attribute__((aligned(A))) = T;
#else
	template <typename T, std::size_t A>
	using aligned_type = T;
#endif

	template <typename T, std::size_t A>
	using aligned_pointer = aligned_type<T, A> *;

	template <typename T, std::size_t A>
	using const_aligned_pointer = const aligned_type<T, A> *;

	template <typename T, std::size_t A>
	using aligned_reference = aligned_type<T, A> &;

	template <typename T, std::size_t A>
	using const_aligned_reference = const aligned_type<T, A> &;
#endif
	//============================================================
	// Standard arena_deleter
	//============================================================

	template <typename P, typename T, bool X>
	struct arena_deleter_base
	{
	};


	template <typename P, typename T>
	struct arena_deleter_base<P, T, false>
	{
		using arena_storage_type = P;

		constexpr arena_deleter_base(arena_storage_type *a = nullptr) noexcept
		: m_a(a) { }

		template<typename U, typename =
			   std::enable_if_t<std::is_convertible< U*, T*>::value>>
		arena_deleter_base(const arena_deleter_base<P, U, false> &rhs) noexcept
		: m_a(rhs.m_a) { }

		void operator()(T *p) noexcept
		{
			// call destructor
			p->~T();
			m_a->deallocate(p, sizeof(T));
		}
	//private:
		arena_storage_type *m_a;
	};

	template <typename P, typename T>
	struct arena_deleter_base<P, T, true>
	{
		using arena_storage_type = P;

		constexpr arena_deleter_base( /*[[maybe_unused]]*/ arena_storage_type *a = nullptr) noexcept
		{
			// gcc 7.2 (mingw) and 7.5 (ubuntu) don't accept maybe_unused here
			plib::unused_var(a);
		}

		template<typename U, typename = typename
			   std::enable_if<std::is_convertible< U*, T*>::value>::type>
		arena_deleter_base( /*[[maybe_unused]]*/ const arena_deleter_base<P, U, true> &rhs) noexcept
		{
			// gcc 7.2 (mingw) and 7.5 (ubuntu) don't accept maybe_unused here
			plib::unused_var(rhs);
		}

		void operator()(T *p) noexcept
		{
			// call destructor
			p->~T();
			P::deallocate(p, sizeof(T));
		}
	};

	template <typename P, typename T>
	struct arena_deleter : public arena_deleter_base<P, T, P::has_static_deallocator>
	{
		using base_type = arena_deleter_base<P, T, P::has_static_deallocator>;
		using base_type::base_type;
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

		owned_ptr(pointer p, bool owned)
		: m_ptr(p), m_deleter(), m_is_owned(owned)
		{ }

		owned_ptr(pointer p, bool owned, D deleter)
		: m_ptr(p), m_deleter(deleter), m_is_owned(owned)
		{ }


		owned_ptr(const owned_ptr &r) = delete;
		owned_ptr & operator =(owned_ptr &r) = delete;

		template<typename DC, typename DC_D>
		owned_ptr & operator =(owned_ptr<DC, DC_D> &&r)  noexcept
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
			m_deleter = r.m_deleter;
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

		///
		/// \brief Return \c true if the stored pointer is not null.
		///
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
		using pointer = T *;
		static /*constexpr*/ const std::size_t align_size = ALIGN;
		using arena_type = ARENA;

		static_assert(align_size >= alignof(T),
			"ALIGN must be greater than alignof(T) and a multiple");
		static_assert((align_size % alignof(T)) == 0,
			"ALIGN must be greater than alignof(T) and a multiple");

		arena_allocator() noexcept
		: m_a(arena_type::instance())
		{ }

		~arena_allocator() noexcept = default;

		arena_allocator(const arena_allocator &) = default;
		arena_allocator &operator=(const arena_allocator &) = default;
		arena_allocator(arena_allocator &&) noexcept = default;
		arena_allocator &operator=(arena_allocator &&) noexcept = default;

		explicit arena_allocator(arena_type & a) noexcept : m_a(a)
		{
		}

		template <class U>
		arena_allocator(const arena_allocator<ARENA, U, ALIGN>& rhs) noexcept
		: m_a(rhs.m_a)
		{
		}

		template <class U>
		struct rebind
		{
			using other = arena_allocator<ARENA, U, ALIGN>;
		};

		pointer allocate(std::size_t n)
		{
			return reinterpret_cast<T *>(m_a.allocate(ALIGN, sizeof(T) * n)); //NOLINT
		}

		void deallocate(pointer p, std::size_t n) noexcept
		{
			m_a.deallocate(p, sizeof(T) * n);
		}

		template<typename U, typename... Args>
		void construct(U* p, Args&&... args)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			::new (void_ptr_cast(p)) U(std::forward<Args>(args)...);
		}

		template<typename U>
		void destroy(U* p)
		{
			p->~U();
		}

		template <class AR1, class T1, std::size_t A1, class AR2, class T2, std::size_t A2>
		friend bool operator==(const arena_allocator<AR1, T1, A1>& lhs, // NOLINT
			const arena_allocator<AR2, T2, A2>& rhs) noexcept;

		template <class AU, class U, std::size_t A>
		friend class arena_allocator;

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

	// MSVC has an issue with SFINAE and overloading resolution.
	// A discussion can be found here:
	//
	// https://stackoverflow.com/questions/31062892/overloading-on-static-in-conjunction-with-sfinae
	//
	// The previous code compiled with gcc and clang on all platforms and
	// compilers apart from MSVC.

	template <typename P, bool HSD, bool HSA>
	struct arena_base;

	template <typename P, bool HSD, bool HSA>
	struct arena_core
	{
		static constexpr const bool has_static_deallocator = HSD;
		static constexpr const bool has_static_allocator = HSA;
		using size_type = std::size_t;

		template <class T, size_type ALIGN = alignof(T)>
		using allocator_type = arena_allocator<P, T, ALIGN>;

		template <class T>
		using deleter_type = arena_deleter<P, T>;

		template <typename T>
		using unique_ptr = std::unique_ptr<T, deleter_type<T>>;

		template <typename T>
		using owned_ptr = plib::owned_ptr<T, deleter_type<T>>;

		static inline P &instance() noexcept
		{
			static P s_arena;
			return s_arena;
		}

		friend struct arena_base<P, HSD, HSA>;
	private:
		size_t m_stat_cur_alloc = 0;
		size_t m_stat_max_alloc = 0;

	};

	template <typename P, bool HSD, bool HSA>
	struct arena_base : public arena_core<P, HSD, HSA>
	{
		using base_type = arena_core<P, HSD, HSA>;
		using size_type = typename base_type::size_type;

		static size_type cur_alloc() noexcept { return base_type::instance().m_stat_cur_alloc; }
		static size_type max_alloc() noexcept { return base_type::instance().m_stat_max_alloc; }

		static inline void inc_alloc_stat(size_type size)
		{
			auto &i = base_type::instance();
			i.m_stat_cur_alloc += size;
			if (i.m_stat_max_alloc <i.m_stat_cur_alloc)
				i.m_stat_max_alloc = i.m_stat_cur_alloc;
		}
		static inline void dec_alloc_stat(size_type size)
		{
			base_type::instance().m_stat_cur_alloc -= size;
		}
	};

	template <typename P>
	struct arena_base<P, false, false> : public arena_core<P, false, false>
	{
		using size_type = typename arena_core<P, false, false>::size_type;

		size_type cur_alloc() const noexcept { return this->m_stat_cur_alloc; }
		size_type max_alloc() const noexcept { return this->m_stat_max_alloc; }

		inline void inc_alloc_stat(size_type size)
		{
			this->m_stat_cur_alloc += size;
			if (this->m_stat_max_alloc < this->m_stat_cur_alloc)
				this->m_stat_max_alloc = this->m_stat_cur_alloc;
		}
		inline void dec_alloc_stat(size_type size)
		{
			this->m_stat_cur_alloc -= size;
		}
	};

	struct aligned_arena : public arena_base<aligned_arena, true, true>
	{
		static inline gsl::owner<void *> allocate( size_t alignment, size_t size )
		{
			inc_alloc_stat(size);

		#if (PUSE_ALIGNED_ALLOCATION)
		#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
			return _aligned_malloc(size, alignment);
		#elif defined(__APPLE__) || defined(__ANDROID__)
			void* p;
			if (::posix_memalign(&p, alignment, size) != 0) {
				p = nullptr;
			}
			return p;
		#else
			return static_cast<gsl::owner<void *>>(aligned_alloc(alignment, size));
		#endif
		#else
			unused_var(alignment);
			return ::operator new(size);
		#endif
		}

		static inline void deallocate(gsl::owner<void *> ptr, size_t size ) noexcept
		{
			//unused_var(size);
			dec_alloc_stat(size);
			#if (PUSE_ALIGNED_ALLOCATION)
				#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
				// NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
				_aligned_free(ptr);
				#else
				// NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
				::free(ptr);
				#endif
			#else
				::operator delete(ptr);
			#endif
		}

		bool operator ==([[maybe_unused]] const aligned_arena &rhs) const noexcept
		{
			return true;
		}

	};

	struct std_arena : public arena_base<std_arena, true, true>
	{
		static inline void *allocate([[maybe_unused]] size_t alignment, size_t size )
		{
			inc_alloc_stat(size);
			return ::operator new(size);
		}

		static inline void deallocate( void *ptr, size_t size ) noexcept
		{
			dec_alloc_stat(size);
			::operator delete(ptr);
		}

		bool operator ==([[maybe_unused]] const aligned_arena &rhs) const noexcept
		{
			return true;
		}
	};

	namespace detail
	{
		template<typename T, typename ARENA, typename... Args>
		static inline T * alloc(Args&&... args)
		{
			auto *mem = ARENA::allocate(alignof(T), sizeof(T));
			try
			{
				// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
				return new (mem) T(std::forward<Args>(args)...);
			}
			catch (...)
			{
				ARENA::deallocate(mem, sizeof(T));
				throw;
			}
		}

		template<typename ARENA, typename T>
		static inline void free(T *ptr) noexcept
		{
			ptr->~T();
			ARENA::deallocate(ptr, sizeof(T));
		}

		template<typename T, typename ARENA, typename... Args>
		static inline T * alloc(ARENA &arena, Args&&... args)
		{
			auto *mem = arena.allocate(alignof(T), sizeof(T));
			try
			{
				// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
				return new (mem) T(std::forward<Args>(args)...);
			}
			catch (...)
			{
				arena.deallocate(mem, sizeof(T));
				throw;
			}
		}

		template<typename ARENA, typename T>
		static inline void free(ARENA &arena, T *ptr) noexcept
		{
			ptr->~T();
			arena.deallocate(ptr, sizeof(T));
		}
	} // namespace detail



	template<typename T, typename ARENA, typename... Args>
	static inline
	std::enable_if_t<ARENA::has_static_allocator, typename ARENA::template unique_ptr<T>>
	make_unique(Args&&... args)
	{
		using up_type = typename ARENA::template unique_ptr<T>;
		using deleter_type = typename ARENA::template deleter_type<T>;
		auto *mem = detail::alloc<T, ARENA>(std::forward<Args>(args)...);
		return up_type(mem, deleter_type());
	}

	template<typename T, typename ARENA, typename... Args>
	static inline
	std::enable_if_t<!ARENA::has_static_allocator, typename ARENA::template unique_ptr<T>>
	make_unique(Args&&... args)
	{
		return make_unique<T>(ARENA::instance(), std::forward<Args>(args)...);
	}

	template<typename T, typename ARENA, typename... Args>
	static inline
	typename ARENA::template unique_ptr<T>
	make_unique(ARENA &arena, Args&&... args)
	{
		using up_type = typename ARENA::template unique_ptr<T>;
		using deleter_type = typename ARENA::template deleter_type<T>;
		auto *mem = detail::alloc<T>(arena, std::forward<Args>(args)...);
		return up_type(mem, deleter_type(&arena));
	}

	template<typename T, typename ARENA, typename... Args>
	static inline
	std::enable_if_t<ARENA::has_static_allocator, typename ARENA::template owned_ptr<T>>
	make_owned(Args&&... args)
	{
		using op_type = typename ARENA::template owned_ptr<T>;
		using deleter_type = typename ARENA::template deleter_type<T>;
		auto *mem = detail::alloc<T, ARENA>(std::forward<Args>(args)...);
		return op_type(mem, true, deleter_type());
	}

	template<typename T, typename ARENA, typename... Args>
	static inline
	std::enable_if_t<!ARENA::has_static_allocator, typename ARENA::template owned_ptr<T>>
	make_owned(Args&&... args)
	{
		return make_owned<T>(ARENA::instance(), std::forward<Args>(args)...);
	}

	template<typename T, typename ARENA, typename... Args>
	static inline typename ARENA::template owned_ptr<T> make_owned(ARENA &arena, Args&&... args)
	{
		using op_type = typename ARENA::template owned_ptr<T>;
		using deleter_type = typename ARENA::template deleter_type<T>;
		auto *mem = detail::alloc<T>(arena, std::forward<Args>(args)...);
		return op_type(mem, true, deleter_type(&arena));
	}


	template <class T, std::size_t ALIGN = alignof(T)>
	using aligned_allocator = aligned_arena::allocator_type<T, ALIGN>;

	//============================================================
	// traits to determine alignment size and stride size
	// from types supporting alignment
	//============================================================

	PDEFINE_HAS_MEMBER(has_align, align_size);

	template <typename T, bool X>
	struct align_traits_base
	{
		static_assert(!has_align<T>::value, "no align");
		static constexpr const std::size_t align_size = alignof(std::max_align_t);
		static constexpr const std::size_t value_size = sizeof(typename T::value_type);
		static constexpr const std::size_t stride_size = lcm(align_size, value_size) / value_size;
	};

	template <typename T>
	struct align_traits_base<T, true>
	{
		static_assert(has_align<T>::value, "no align");
		static constexpr const std::size_t align_size = T::align_size;
		static constexpr const std::size_t value_size = sizeof(typename T::value_type);
		static constexpr const std::size_t stride_size = lcm(align_size, value_size) / value_size;
	};

	template <typename T>
	struct align_traits : public align_traits_base<T, has_align<T>::value>
	{};

	template <typename BASEARENA = aligned_arena, std::size_t PG_SIZE = 1024>
	class paged_arena : public arena_base<paged_arena<BASEARENA, PG_SIZE>, true, true>
	{
	public:
		paged_arena() = default;

		PCOPYASSIGNMOVE(paged_arena, delete)

		~paged_arena() = default;

		static void *allocate([[maybe_unused]] size_t align, size_t size)
		{
			//size = ((size + PG_SIZE - 1) / PG_SIZE) * PG_SIZE;
			return arena().allocate(PG_SIZE, size);
		}

		static void deallocate(void *ptr, size_t size) noexcept
		{
			//size = ((size + PG_SIZE - 1) / PG_SIZE) * PG_SIZE;
			arena().deallocate(ptr, size);
		}

		bool operator ==(const paged_arena &rhs) const noexcept { return this == &rhs; }

		static BASEARENA &arena() noexcept { static BASEARENA m_arena; return m_arena; }
	private:
	};

	//============================================================
	// Aligned vector
	//============================================================

	// FIXME: needs a separate file
	template <typename T, std::size_t ALIGN = PALIGN_VECTOROPT, typename A = paged_arena<>>//aligned_arena>
	class aligned_vector : public std::vector<T, typename A::template allocator_type<T, ALIGN>>
	{
	public:
		using base = std::vector<T, typename A::template allocator_type<T, ALIGN>>;

		using base::base;

	};

} // namespace plib

#endif // PALLOC_H_
