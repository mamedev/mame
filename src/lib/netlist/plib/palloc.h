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
	// Standard arena_deleter
	//============================================================

	template <typename P, typename T, std::size_t ALIGN, bool X>
	struct arena_deleter_base
	{
	};

	struct deleter_info_t
	{
		std::size_t alignment;
		std::size_t size;
	};

	template <typename P, typename T, std::size_t ALIGN>
	struct arena_deleter_base<P, T, ALIGN, false>
	{
		constexpr arena_deleter_base(P *a = nullptr) noexcept
		: m_arena(a), m_info({ALIGN ? ALIGN : alignof(T), sizeof(T)} ) { }

		template<typename U, typename =
		std::enable_if_t<std::is_convertible_v<U *, T *>>>
		constexpr arena_deleter_base(const arena_deleter_base<P, U, ALIGN, false> &rhs) noexcept
		: m_arena(rhs.m_arena), m_info(rhs.m_info) { }

		void operator()(T *p) noexcept
		{
			// call destructor
			p->~T();
			m_arena->deallocate(p, m_info.alignment, m_info.size);
		}

		P *m_arena;
		deleter_info_t m_info;
	};

	template <typename P, typename T, std::size_t ALIGN>
	struct arena_deleter_base<P, T, ALIGN, true>
	{
		constexpr arena_deleter_base(/*[[maybe_unused]]*/ P *a = nullptr) noexcept
		: m_info({ALIGN ? ALIGN : alignof(T), sizeof(T)})
		{
			plib::unused_var(a); // GCC 7.x does not like the maybe_unused
		}

		template<typename U, typename =
		std::enable_if_t<std::is_convertible_v<U *, T *>>>
		constexpr arena_deleter_base(const arena_deleter_base<P, U, ALIGN, true> &rhs) noexcept
		: m_info(rhs.m_info)
		{
		}

		void operator()(T *p) noexcept
		{
			// call destructor
			p->~T();
			P::deallocate(p, m_info.alignment, m_info.size);
		}

		deleter_info_t m_info;
	};


	///
	/// \brief alignment aware deleter class
	///
	/// The deleter class expects the object to have been allocated with
	/// `alignof(T)` alignment if the ALIGN parameter is omitted. If ALIGN is
	/// given, this is used.
	///
	/// \tparam A Arena type
	/// \tparam T Object type
	/// \tparam ALIGN alignment
	///
	template <typename A, typename T, std::size_t ALIGN = 0>
	struct arena_deleter : public arena_deleter_base<A, T, ALIGN, A::has_static_deallocator>
	{
		using base_type = arena_deleter_base<A, T, ALIGN, A::has_static_deallocator>;
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
		: m_ptr(p), m_deleter(std::move(deleter)), m_is_owned(owned)
		{ }


		owned_ptr(const owned_ptr &r) = delete;
		owned_ptr & operator =(owned_ptr &r) = delete;

		template <typename DC, typename DC_D>
		owned_ptr & operator =(owned_ptr<DC, DC_D> &&r)  noexcept
		{
			if (m_is_owned && (m_ptr != nullptr))
				m_deleter(m_ptr);
			m_is_owned = r.m_is_owned;
			m_ptr = r.m_ptr;
			m_deleter = std::move(r.m_deleter);
			r.m_is_owned = false;
			r.m_ptr = nullptr;
			return *this;
		}

		owned_ptr(owned_ptr &&r) noexcept
		: m_ptr(r.m_ptr)
		, m_deleter(std::move(r.m_deleter))
		, m_is_owned(r.m_is_owned)
		{
			r.m_is_owned = false;
			r.m_ptr = nullptr;
		}

		owned_ptr &operator=(owned_ptr &&r) noexcept
		{
			if (m_is_owned && (m_ptr != nullptr))
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
		, m_deleter(std::move(r.m_deleter))
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

	template <class ARENA, class T, std::size_t ALIGN, bool HSA>
	class arena_allocator
	{
	public:
		using value_type = T;
		using pointer = T *;
		static constexpr const std::size_t align_size = ALIGN ? ALIGN : alignof(T);
		using arena_type = ARENA;

		static_assert((align_size % alignof(T)) == 0,
			"ALIGN must be greater than alignof(T) and a multiple");

		template <typename U = int, typename = std::enable_if_t<HSA && sizeof(U)>>
		//[[deprecated]]
		arena_allocator() noexcept
		: m_a(arena_type::instance())
		{ }

		~arena_allocator() noexcept = default;

		arena_allocator(const arena_allocator &) = default;
		arena_allocator &operator=(const arena_allocator &) = default;
		arena_allocator(arena_allocator &&) noexcept = default;
		arena_allocator &operator=(arena_allocator &&) noexcept = default;

		template <typename U = int>
		arena_allocator(/*[[maybe_unused]]*/ std::enable_if_t<HSA && sizeof(U), arena_type> & a) noexcept
		: m_a(arena_type::instance())
		{
			plib::unused_var(a); // GCC 7.x does not like the maybe_unused
		}

		template <typename U = int>
		arena_allocator(/*[[maybe_unused]]*/ std::enable_if_t<!HSA && sizeof(U), arena_type> & a) noexcept
		: m_a(a)
		{
			plib::unused_var(a); // GCC 7.x does not like the maybe_unused
		}

		template <class U, typename = std::enable_if_t<!std::is_same_v<T, U>>>
		arena_allocator(const arena_allocator<ARENA, U, ALIGN, HSA>& rhs) noexcept
		: m_a(rhs.m_a)
		{
		}

		template <class U>
		struct rebind
		{
			using other = arena_allocator<ARENA, U, ALIGN, HSA>;
		};

		pointer allocate(std::size_t n)
		{
			return reinterpret_cast<T *>(m_a.allocate(align_size, sizeof(T) * n)); //NOLINT
		}

		void deallocate(pointer p, std::size_t n) noexcept
		{
			m_a.deallocate(p, align_size, sizeof(T) * n);
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

		template <class AR1, class T1, std::size_t A1, bool HSA1, class AR2, class T2, std::size_t A2, bool HSA2>
		friend bool operator==(const arena_allocator<AR1, T1, A1, HSA1>& lhs, // NOLINT
			const arena_allocator<AR2, T2, A2, HSA2>& rhs) noexcept;

		template <class AU1, class U1, std::size_t A1, bool HSA1>
		friend class arena_allocator;

	private:
		arena_type &m_a;
	};

	template <class AR1, class T1, std::size_t A1, bool HSA1, class AR2, class T2, std::size_t A2, bool HSA2>
	inline bool operator==(const arena_allocator<AR1, T1, A1, HSA1>& lhs,
		const arena_allocator<AR2, T2, A2, HSA2>& rhs) noexcept
	{
		return A1 == A2 && rhs.m_a == lhs.m_a;
	}
	template <class AR1, class T1, std::size_t A1, bool HSA1,class AR2, class T2, std::size_t A2, bool HSA2>
	inline bool operator!=(const arena_allocator<AR1, T1, A1, HSA1>& lhs,
		const arena_allocator<AR2, T2, A2, HSA2>& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	//============================================================
	//  Memory allocation
	//============================================================

	//template <typename P, std::size_t MINALIGN, bool HSD, bool HSA>
	//struct arena_base;

	template <typename P, std::size_t MINALIGN, bool HSD, bool HSA>
	struct arena_core
	{
		static constexpr const bool has_static_deallocator = HSD;
		static constexpr const bool has_static_allocator = HSA;
		static constexpr const std::size_t min_align = MINALIGN;
		using size_type = std::size_t;

		template <class T, size_type ALIGN = 0>
		using allocator_type = arena_allocator<P, T, (ALIGN < MINALIGN) ? MINALIGN : ALIGN, HSA>;

		template <class T, size_type ALIGN = 0>
		using deleter_type = arena_deleter<P, T, (ALIGN < MINALIGN) ? MINALIGN : ALIGN>;

		template <typename T, size_type ALIGN = 0>
		using unique_ptr = std::unique_ptr<T, deleter_type<T, (ALIGN < MINALIGN) ? MINALIGN : ALIGN>>;

		template <typename T, size_type ALIGN = 0>
		using owned_ptr = plib::owned_ptr<T, deleter_type<T, (ALIGN < MINALIGN) ? MINALIGN : ALIGN>>;

		static P &instance() noexcept;

		template <class T, size_type ALIGN = 0>
		allocator_type<T, ALIGN> get_allocator()
		{
			return *static_cast<P *>(this);
		}

	protected:
		size_t m_stat_cur_alloc = 0;
		size_t m_stat_max_alloc = 0;

	};

	template <typename P, std::size_t MINALIGN, bool HSD, bool HSA>
	inline P & arena_core<P, MINALIGN, HSD, HSA>::instance() noexcept
	{
		static P s_arena;
		return s_arena;
	}

	template <typename P, std::size_t MINALIGN, bool HSD, bool HSA>
	struct arena_base : public arena_core<P, MINALIGN, HSD, HSA>
	{
		using base_type = arena_core<P, MINALIGN, HSD, HSA>;
		using size_type = typename base_type::size_type;

		~arena_base()
		{
			//printf("%s %lu %lu %lu\n", typeid(*this).name(), MINALIGN, cur_alloc(), max_alloc());
		}

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

	template <typename P, std::size_t MINALIGN>
	struct arena_base<P, MINALIGN, false, false> : public arena_core<P, MINALIGN, false, false>
	{
		using size_type = typename arena_core<P, MINALIGN, false, false>::size_type;

		~arena_base()
		{
			//printf("%s %lu %lu %lu\n", typeid(*this).name(), MINALIGN, cur_alloc(), max_alloc());
		}

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

	template <std::size_t MINALIGN>
	struct aligned_arena : public arena_base<aligned_arena<MINALIGN>, MINALIGN, true, true>
	{
		using base_type = arena_base<aligned_arena<MINALIGN>, MINALIGN, true, true>;

		static inline gsl::owner<void *> allocate( size_t alignment, size_t size )
		{
			base_type::inc_alloc_stat(size);
#if 0
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
			// see https://en.cppreference.com/w/c/memory/aligned_alloc
			size = ((size + alignment - 1) / alignment) * alignment;
			return static_cast<gsl::owner<void *>>(aligned_alloc(alignment, size));
		#endif
		#else
			unused_var(alignment);
			return ::operator new(size);
		#endif
#else
			return ::operator new(size, std::align_val_t(alignment));
#endif
		}

		static inline void deallocate(gsl::owner<void *> ptr, [[maybe_unused]] size_t alignment, size_t size ) noexcept
		{
			//unused_var(size);
			base_type::dec_alloc_stat(size);
#if 0
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
#else
			::operator delete(ptr, std::align_val_t(alignment));
#endif
		}

		bool operator ==([[maybe_unused]] const aligned_arena &rhs) const noexcept
		{
			return true;
		}

	};

	struct std_arena : public arena_base<std_arena, 0, true, true>
	{
		static inline void *allocate(size_t alignment, size_t size )
		{
			inc_alloc_stat(size);
			return ::operator new(size, static_cast<std::align_val_t>(alignment));
		}

		static inline void deallocate( void *ptr, size_t alignment, size_t size ) noexcept
		{
			dec_alloc_stat(size);
			::operator delete(ptr, static_cast<std::align_val_t>(alignment));
		}

		bool operator ==([[maybe_unused]] const std_arena &rhs) const noexcept
		{
			return true;
		}
	};

	namespace detail
	{
		///
		/// \brief Create new object T with an aligned memory
		///
		/// The create object can be deallocate using \ref free or
		/// using the arena_deleter type. This is the specialization for arenas
		/// which have no state.
		///
		/// \tparam T      Object type
		/// \tparam ALIGN  Alignment of object to be created. If ALIGN equals 0, alignof(T) is used.
		/// \tparam ARENA  Arena type
		/// \tparam Args   Argument types
		///
		/// \param args    Arguments to be passed to constructor
		///
		template<typename T, std::size_t ALIGN, typename ARENA, typename... Args>
		static inline T * alloc(Args&&... args)
		{
			//using alloc_type = typename ARENA :: template allocator_type<T, alignof(T)>;
			auto *mem = ARENA::allocate(ALIGN ? ALIGN : alignof(T), sizeof(T));
			try
			{
				// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
				return new (mem) T(std::forward<Args>(args)...);
			}
			catch (...)
			{
				ARENA::deallocate(mem, ALIGN ? ALIGN : alignof(T), sizeof(T));
				throw;
			}
		}

		template<std::size_t ALIGN, typename ARENA, typename T>
		static inline void free(T *ptr) noexcept
		{
			ptr->~T();
			ARENA::deallocate(ptr, ALIGN ? ALIGN : alignof(T), sizeof(T));
		}

		///
		/// \brief Create new object T with an aligned memory
		///
		/// The create object can be deallocate using \ref free or
		/// using the arena_deleter type. This is the specialization for arenas
		/// which do have state.
		///
		/// \tparam T      Object type
		/// \tparam ALIGN  Alignment of object to be created. If ALIGN equals 0, alignof(T) is used.
		/// \tparam ARENA  Arena type
		/// \tparam Args   Argument types
		///
		/// \param arena   Arena to provide memory
		/// \param args    Arguments to be passed to constructor
		///
		template<typename T, std::size_t ALIGN, typename ARENA, typename... Args>
		static inline T * alloc(ARENA &arena, Args&&... args)
		{
			auto *mem = arena.allocate(ALIGN ? ALIGN : alignof(T), sizeof(T));
			try
			{
				// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
				return new (mem) T(std::forward<Args>(args)...);
			}
			catch (...)
			{
				arena.deallocate(mem, ALIGN ? ALIGN : alignof(T), sizeof(T));
				throw;
			}
		}

		template<std::size_t ALIGN, typename ARENA, typename T>
		static inline void free(ARENA &arena, T *ptr) noexcept
		{
			ptr->~T();
			arena.deallocate(ptr, ALIGN ? ALIGN : alignof(T), sizeof(T));
		}
	} // namespace detail


	///
	/// \brief Create new alignment and size aware std::unique_ptr
	///
	/// `make_unique` creates a new shared pointer to the object it creates.
	/// These version ensure that on deallocation the correct alignment and
	/// size is used. Should the unique_ptr be down casted the deleter objects
	/// used here track the size and alignment of the object created.
	///
	/// std::standard_delete will use size and alignment of the base class.
	///
	/// This function is deprecated since it hides the use of arenas.
	///
	/// \tparam T      Object type
	/// \tparam ARENA  Arena type
	/// \tparam ALIGN  Alignment of object to be created. If ALIGN equals 0, alignof(T) is used.
	/// \tparam Args   Argument types
	///
	/// \param args    Arguments to be passed to constructor
	///
	template<typename T, typename ARENA, std::size_t ALIGN = 0, typename... Args>
	//[[deprecated]]
	std::enable_if_t<ARENA::has_static_allocator, typename ARENA::template unique_ptr<T, ALIGN>>
	make_unique(Args&&... args)
	{
		using up_type = typename ARENA::template unique_ptr<T, ALIGN>;
		using deleter_type = typename ARENA::template deleter_type<T, ALIGN>;
		auto *mem = detail::alloc<T, ALIGN, ARENA>(std::forward<Args>(args)...);
		return up_type(mem, deleter_type());
	}

	///
	/// \brief Create new alignment and size aware std::unique_ptr
	///
	/// `make_unique` creates a new shared pointer to the object it creates.
	/// These version ensure that on deallocation the correct alignment and
	/// size is used. Should the unique_ptr be down casted the deleter objects
	/// used here track the size and alignment of the object created.
	///
	/// std::standard_delete will use size and alignment of the base class.
	///
	/// \tparam T      Object type
	/// \tparam ARENA  Arena type
	/// \tparam ALIGN  Alignment of object to be created. If ALIGN equals 0, alignof(T) is used.
	/// \tparam Args   Argument types
	///
	/// \param arena   Arena to provide memory
	/// \param args    Arguments to be passed to constructor
	///
	template<typename T, typename ARENA, std::size_t ALIGN = 0, typename... Args>
	typename ARENA::template unique_ptr<T, ALIGN>
	make_unique(ARENA &arena, Args&&... args)
	{
		using up_type = typename ARENA::template unique_ptr<T, ALIGN>;
		using deleter_type = typename ARENA::template deleter_type<T, ALIGN>;
		auto *mem = detail::alloc<T, ALIGN>(arena, std::forward<Args>(args)...);
		return up_type(mem, deleter_type(&arena));
	}

	///
	/// \brief Create new alignment and size aware plib::owned_ptr
	///
	/// `make_unique` creates a new shared pointer to the object it creates.
	/// These version ensure that on deallocation the correct alignment and
	/// size is used. Should the unique_ptr be down casted the deleter objects
	/// used here track the size and alignment of the object created.
	///
	/// std::standard_delete will use size and alignment of the base class.
	///
	/// This function is deprecated since it hides the use of arenas.
	///
	/// \tparam T      Object type
	/// \tparam ARENA  Arena type
	/// \tparam ALIGN  Alignment of object to be created. If ALIGN equals 0, alignof(T) is used.
	/// \tparam Args   Argument types
	///
	/// \param args    Arguments to be passed to constructor
	///
	template<typename T, typename ARENA, std::size_t ALIGN = 0, typename... Args>
	//[[deprecated]]
	std::enable_if_t<ARENA::has_static_allocator, typename ARENA::template owned_ptr<T, ALIGN>>
	make_owned(Args&&... args)
	{
		using op_type = typename ARENA::template owned_ptr<T, ALIGN>;
		using deleter_type = typename ARENA::template deleter_type<T, ALIGN>;
		auto *mem = detail::alloc<T, ALIGN, ARENA>(std::forward<Args>(args)...);
		return op_type(mem, true, deleter_type());
	}

	///
	/// \brief Create new alignment and size aware plib::owned_ptr
	///
	/// `make_unique` creates a new shared pointer to the object it creates.
	/// These version ensure that on deallocation the correct alignment and
	/// size is used. Should the unique_ptr be down casted the deleter objects
	/// used here track the size and alignment of the object created.
	///
	/// std::standard_delete will use size and alignment of the base class.
	///
	/// \tparam T      Object type
	/// \tparam ARENA  Arena type
	/// \tparam ALIGN  Alignment of object to be created. If ALIGN equals 0, alignof(T) is used.
	/// \tparam Args   Argument types
	///
	/// \param arena   Arena to provide memory
	/// \param args    Arguments to be passed to constructor
	///
	template<typename T, typename ARENA, std::size_t ALIGN = 0, typename... Args>
	typename ARENA::template owned_ptr<T> make_owned(ARENA &arena, Args&&... args)
	{
		using op_type = typename ARENA::template owned_ptr<T, ALIGN>;
		using deleter_type = typename ARENA::template deleter_type<T, ALIGN>;
		auto *mem = detail::alloc<T, ALIGN>(arena, std::forward<Args>(args)...);
		return op_type(mem, true, deleter_type(&arena));
	}

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

	///
	/// \brief Force BASEARENA to align memory allocations on page boundaries
	///
	/// \tparam BASEARENA The base arena to use (optional, defaults to aligned_arena)
	/// \tparam PG_SIZE The page size to use (optional, defaults to 1024)
	///
	template <template<std::size_t> class BASEARENA = aligned_arena, std::size_t PG_SIZE = 1024>
	using paged_arena = BASEARENA<PG_SIZE>;

	///
	/// \brief Helper class to create arena versions of standard library sequences
	///
	/// \ref arena_vector on how to use this class
	///
	/// \tparam A Arena typeThe base arena to use (optional, defaults to aligned_arena)
	/// \tparam T Object type of objects in sequence
	/// \tparam S Sequence, e.g. std::vector, std::list
	/// \tparam ALIGN Alignment to use
	///
	template <typename A, typename T, template <typename, typename> class S, std::size_t ALIGN = PALIGN_VECTOROPT>
	class arena_sequence : public S<T, typename A::template allocator_type<T, ALIGN>>
	{
	public:
		using arena_allocator_type = typename A::template allocator_type<T, ALIGN>;
		using arena_sequence_base = S<T, arena_allocator_type>;
		using arena_sequence_base::arena_sequence_base;

		using size_type = typename arena_sequence_base::size_type;

		arena_sequence(A &arena)
		: arena_sequence_base(seq_alloc(arena))
		{
		}

		arena_sequence(A &arena, size_type n)
		: arena_sequence_base(n, seq_alloc(arena))
		{
		}

	private:
		arena_allocator_type seq_alloc(A &arena) const { return arena.template get_allocator<T, ALIGN>(); }
	};

	///
	/// \brief Vector with arena allocations
	///
	/// The Vector allocation will use the arena of type A of which an instance
	/// has to be passed to the constructor. Should the minimum alignment exceed
	/// the object size, min_align / sizeof(T) elements are reserved.
	///
	/// \tparam A Arena type
	/// \tparam T Object type of objects in sequence
	/// \tparam ALIGN Alignment to use
	///
	template <typename A, typename T, std::size_t ALIGN = PALIGN_VECTOROPT>
	class arena_vector : public arena_sequence<A, T, std::vector, ALIGN>
	{
	public:
		using arena_vector_base = arena_sequence<A, T, std::vector, ALIGN>;
		using arena_vector_base::arena_vector_base;

		///
		/// \brief Constructor
		///
		/// \param arena Arena instance to use
		///
		arena_vector(A &arena)
		: arena_vector_base(arena)
		{
			if (A::min_align / sizeof(T) > 0)
				this->reserve(A::min_align / sizeof(T));
		}

	};

} // namespace plib

#endif // PALLOC_H_
