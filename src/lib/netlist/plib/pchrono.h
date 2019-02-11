// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pchrono.h
 *
 */

#ifndef PCHRONO_H_
#define PCHRONO_H_

#include "pconfig.h"
#include "ptypes.h"

#include <chrono>
#include <cstdint>

namespace plib {
namespace chrono {
	template <typename T>
	struct sys_ticks
	{
		using type = typename T::rep;
		static inline type start() { return T::now().time_since_epoch().count(); }
		static inline type stop() { return T::now().time_since_epoch().count(); }
		static inline constexpr type per_second() { return T::period::den / T::period::num; }
	};

	using hires_ticks = sys_ticks<std::chrono::high_resolution_clock>;
	using steady_ticks = sys_ticks<std::chrono::steady_clock>;
	using system_ticks = sys_ticks<std::chrono::system_clock>;

	#if defined(__x86_64__) &&  !defined(_clang__) && !defined(_MSC_VER) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))

	#if PHAS_RDTSCP
	struct fast_ticks
	{
		typedef int64_t type;
		static inline type start()
		{
			int64_t v;
			__asm__ __volatile__ (
					"rdtscp;"
					"shl $32, %%rdx;"
					"or %%rdx, %%rax;"
					: "=a"(v)           /* outputs  */
					:                   /* inputs   */
					: "%rcx", "%rdx"    /* clobbers */
			);
			return v;
		}
		static inline type stop()
		{
			return start();
		}
		static type per_second();
	};

	#else
	struct fast_ticks
	{
		typedef int64_t type;
		static inline type start()
		{
			int64_t v;
			__asm__ __volatile__ (
					"rdtsc;"
					"shl $32, %%rdx;"
					"or %%rdx, %%rax;"
					: "=a"(v)   /* outputs  */
					:           /* inputs   */
					: "%rdx"    /* clobbers */
			);
			return v;
		}
		static inline type stop()
		{
			return start();
		}
		static type per_second();
	};

	#endif


	/* Based on "How to Benchmark Code Execution Times on Intel?? IA-32 and IA-64
	 * Instruction Set Architectures", Intel, 2010
	 *
	 */
	#if PUSE_ACCURATE_STATS && PHAS_RDTSCP
	/*
	 * kills performance completely, but is accurate
	 * cpuid serializes, but clobbers ebx and ecx
	 *
	 */

	struct exact_ticks
	{
		typedef int64_t type;

		static inline type start()
		{
			int64_t v;
			__asm__ __volatile__ (
					"cpuid;"
					//"xor %%eax, %%eax\n\t"
					"rdtsc;"
					"shl $32, %%rdx;"
					"or %%rdx, %%rax;"
					: "=a"(v) /* outputs */
					: "a"(0x0)                /* inputs */
					: "%ebx", "%ecx", "%rdx"  /* clobbers*/
			);
			return v;
		}
		static inline type stop()
		{
			int64_t v;
			__asm__ __volatile__ (
					"rdtscp;"
					"shl $32, %%rdx;"
					"or %%rax, %%rdx;"
					"mov %%rdx, %%r10;"
					"xor %%eax, %%eax\n\t"
					"cpuid;"
					"mov %%r10, %%rax;"
					: "=a" (v)
					:
					: "%ebx", "%ecx", "%rdx", "%r10"
			);
			return v;
		}

		static type per_second();
	};
	#else
	using exact_ticks = fast_ticks;
	#endif


	#else
	using fast_ticks = hires_ticks;
	using exact_ticks = fast_ticks;
	#endif

	template<bool enabled_>
	struct counter
	{
		counter() : m_count(0) { }
		using type = uint_least64_t;
		type operator()() const { return m_count; }
		void inc() { ++m_count; }
		void reset() { m_count = 0; }
		constexpr static bool enabled = enabled_;
	private:
		type m_count;
	};

	template<>
	struct counter<false>
	{
		using type = uint_least64_t;
		constexpr type operator()() const { return 0; }
		void inc() const { }
		void reset() const { }
		constexpr static bool enabled = false;
	};


	template< typename T, bool enabled_ = true>
	struct timer
	{
		using type = typename T::type;
		using ctype = uint_least64_t;
		constexpr static bool enabled = enabled_;

		struct guard_t
		{
			guard_t() = delete;
			guard_t(timer &m) noexcept : m_m(m) { m_m.m_time -= T::start(); }
			~guard_t() { m_m.m_time += T::stop(); ++m_m.m_count; }

			COPYASSIGNMOVE(guard_t, default)

		private:
			timer &m_m;
		};

		friend struct guard_t;

		timer() : m_time(0), m_count(0) { }

		type operator()() const { return m_time; }

		void reset() { m_time = 0; m_count = 0; }
		type average() const { return (m_count == 0) ? 0 : m_time / m_count; }
		type total() const { return m_time; }
		ctype count() const { return m_count; }

		double as_seconds() const { return static_cast<double>(total())
				/ static_cast<double>(T::per_second()); }

		guard_t guard() { return guard_t(*this); }
	private:
		type m_time;
		ctype m_count;
	};

	template<typename T>
	struct timer<T, false>
	{
		using type = typename T::type;
		using ctype = uint_least64_t;

		struct guard_t
		{
			guard_t() = default;
			COPYASSIGNMOVE(guard_t, default)
			/* using default constructor will trigger warning on
			 * unused local variable.
			 */
			// NOLINTNEXTLINE(modernize-use-equals-default)
			~guard_t() { }
		};

		constexpr type operator()() const { return 0; }
		void reset() const { }
		constexpr type average() const { return 0; }
		constexpr type total() const { return 0; }
		constexpr ctype count() const { return 0; }
		constexpr double as_seconds() const { return 0.0; }
		constexpr static bool enabled = false;
		guard_t guard() { return guard_t(); }
	};


	} // namespace chrono
} // namespace plib

#endif /* PCHRONO_H_ */
