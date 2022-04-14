// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PCHRONO_H_
#define PCHRONO_H_

///
/// \file pchrono.h
///

#include "pconfig.h"
#include "pgsl.h"
#include "ptypes.h"

#include <chrono>

namespace plib {
	namespace chrono {
		template <typename T>
		struct sys_ticks
		{
			using type = typename T::rep;
			static constexpr type start() noexcept { return T::now().time_since_epoch().count(); }
			static constexpr type stop() noexcept { return T::now().time_since_epoch().count(); }
			static constexpr type per_second() noexcept { return T::period::den / T::period::num; }
		};

		using hires_ticks = sys_ticks<std::chrono::high_resolution_clock>;
		using steady_ticks = sys_ticks<std::chrono::steady_clock>;
		using system_ticks = sys_ticks<std::chrono::system_clock>;

		#if defined(__x86_64__) &&  !defined(_clang__) && !defined(_MSC_VER) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))

		template <typename T, typename R>
		struct base_ticks
		{
			using ret_type = R;
			static ret_type per_second() noexcept
			{
				static ret_type persec = 0;
				if (persec == 0)
				{
					ret_type x = 0;
					system_ticks::type t = system_ticks::start();
					system_ticks::type e;
					x = - T :: start();
					do {
						e = system_ticks::stop();
					} while (e - t < system_ticks::per_second() / 100 );
					x += T :: stop();
					persec = (ret_type)(double)((double) x * (double) system_ticks::per_second() / double (e - t));
				}
				return persec;
			}
		};


		#if PHAS_RDTSCP
		struct fast_ticks : public base_ticks<fast_ticks, int64_t>
		{
			typedef int64_t type;
			static inline type start() noexcept
			{
				int64_t v;
				__asm__ __volatile__ (
						"rdtscp;"
						"shl $32, %%rdx;"
						"or %%rdx, %%rax;"
						: "=a"(v)           // outputs
						:                   // inputs
						: "%rcx", "%rdx"    // clobbers
				);
				return v;
			}
			static inline type stop() noexcept
			{
				return start();
			}
		};

		#else
		struct fast_ticks : public base_ticks<fast_ticks, int64_t>
		{
			typedef int64_t type;
			static inline type start() noexcept
			{
				int64_t v;
				__asm__ __volatile__ (
						"rdtsc;"
						"shl $32, %%rdx;"
						"or %%rdx, %%rax;"
						: "=a"(v)   // outputs
						:           // inputs
						: "%rdx"    // clobbers
				);
				return v;
			}
			static inline type stop() noexcept
			{
				return start();
			}
		};

		#endif


		// Based on "How to Benchmark Code Execution Times on Intel?? IA-32 and IA-64
		// Instruction Set Architectures", Intel, 2010

		#if PUSE_ACCURATE_STATS && PHAS_RDTSCP
		//
		// kills performance completely, but is accurate
		// cpuid serializes, but clobbers ebx and ecx
		//

		struct exact_ticks : public base_ticks<exact_ticks, int64_t>
		{
			typedef int64_t type;

			static inline type start() noexcept
			{
				int64_t v;
				__asm__ __volatile__ (
						"cpuid;"
						//"xor %%eax, %%eax\n\t"
						"rdtsc;"
						"shl $32, %%rdx;"
						"or %%rdx, %%rax;"
						: "=a"(v) // outputs
						: "a"(0x0)                // inputs
						: "%ebx", "%ecx", "%rdx"  // clobbers
				);
				return v;
			}
			static inline type stop() noexcept
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
			type operator()() const noexcept { return m_count; }
			void inc() noexcept { ++m_count; }
			void reset() noexcept { m_count = 0; }
			constexpr static bool enabled = enabled_;
		private:
			type m_count;
		};

		template<>
		struct counter<false>
		{
			using type = uint_least64_t;
			constexpr type operator()() const noexcept { return 0; }
			void inc() const noexcept { }
			void reset() const noexcept { }
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
				explicit constexpr guard_t(timer &m) noexcept : m_m(&m) { m_m->m_time -= T::start(); }
				~guard_t() noexcept { m_m->m_time += T::stop(); ++m_m->m_count; }

				constexpr guard_t(const guard_t &) = default;
				constexpr guard_t &operator=(const guard_t &) = default;
				constexpr guard_t(guard_t &&) noexcept = default;
				constexpr guard_t &operator=(guard_t &&) noexcept = default;

			private:
				timer *m_m;
			};

			constexpr timer() : m_time(0), m_count(0) { }

			constexpr type operator()() const { return m_time; }

			void reset() noexcept { m_time = 0; m_count = 0; }
			constexpr type average() const noexcept { return (m_count == 0) ? 0 : m_time / m_count; }
			constexpr type total() const noexcept { return m_time; }
			constexpr ctype count() const noexcept { return m_count; }

			template <typename S>
			constexpr S as_seconds() const noexcept { return narrow_cast<S>(total())
					/ narrow_cast<S>(T::per_second()); }

			constexpr guard_t guard() noexcept { return guard_t(*this); }

			// pause must be followed by cont(inue)
			void stop() noexcept { m_time += T::stop(); }
			void start() noexcept { m_time -= T::start(); }


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
				constexpr guard_t() = default;

				constexpr guard_t(const guard_t &) = default;
				constexpr guard_t &operator=(const guard_t &) = default;
				constexpr guard_t(guard_t &&) noexcept = default;
				constexpr guard_t &operator=(guard_t &&) noexcept = default;

				// using default constructor will trigger warning on
				// unused local variable.

				// NOLINTNEXTLINE(modernize-use-equals-default)
				~guard_t() noexcept { }
			};

			constexpr type operator()() const noexcept { return 0; }
			void reset() const noexcept { }
			constexpr type average() const noexcept { return 0; }
			constexpr type total() const noexcept { return 0; }
			constexpr ctype count() const noexcept { return 0; }
			template <typename S>
			constexpr S as_seconds() const noexcept { return narrow_cast<S>(0); }
			constexpr static bool enabled = false;
			guard_t guard() { return guard_t(); }
		};

	} // namespace chrono
	//============================================================
	//  Performance tracking
	//============================================================

	template<bool enabled_>
	using pperftime_t = plib::chrono::timer<plib::chrono::exact_ticks, enabled_>;

	template<bool enabled_>
	using pperfcount_t = plib::chrono::counter<enabled_>;
} // namespace plib

#endif // PCHRONO_H_
