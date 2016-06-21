// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pchrono.h
 *
 */

#ifndef PCHRONO_H_
#define PCHRONO_H_

#include <cstdint>
#include <thread>
#include <chrono>

#include "pconfig.h"

namespace plib {
namespace chrono {
	template <typename T>
	struct sys_ticks
	{
		typedef typename T::rep type;
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
					: "=a"(v) 			/* outputs  */
					: 			        /* inputs   */
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
					: "=a"(v) 	/* outputs  */
					: 			/* inputs   */
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


	/* Based on "How to Benchmark Code Execution Times on Intel® IA-32 and IA-64
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
		typedef uint_least64_t type;
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
		typedef uint_least64_t type;
		constexpr type operator()() const { return 0; }
		void inc() const { }
		void reset() const { }
		constexpr static bool enabled = false;
	};


	template< typename T, bool enabled_ = true>
	struct timer
	{
		typedef typename T::type type;
		typedef uint_least64_t   ctype;

		timer() : m_time(0), m_count(0) { }

		type operator()() const { return m_time; }

		void start() { m_time -= T::start(); }
		void stop() { m_time += T::stop(); ++m_count; }
		void reset() { m_time = 0; m_count = 0; }
		type average() const { return (m_count == 0) ? 0 : m_time / m_count; }
		type total() const { return m_time; }
		ctype count() const { return m_count; }

		double as_seconds() const { return (double) total() / (double) T::per_second(); }

		constexpr static bool enabled = enabled_;
	private:
		type m_time;
		ctype m_count;
	};

	template<typename T>
	struct timer<T, false>
	{
		typedef typename T::type type;
		typedef uint_least64_t   ctype;
		constexpr type operator()() const { return 0; }
		void start()  const { }
		void stop() const { }
		void reset() const { }
		constexpr type average() const { return 0; }
		constexpr type total() const { return 0; }
		constexpr ctype count() const { return 0; }
		constexpr double as_seconds() const { return 0.0; }
		constexpr static bool enabled = false;
	};

	} // namespace chrono
} // namespace plib

#endif /* PCHRONO_H_ */
