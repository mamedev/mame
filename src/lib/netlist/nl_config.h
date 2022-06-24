// license:BSD-3-Clause
// copyright-holders:Couriersud

/// \file nl_config.h
///

#ifndef NLCONFIG_H_
#define NLCONFIG_H_

// Names
// spell-checker: words Woodbury

#include "plib/pconfig.h"
#include "plib/pexception.h"

#include <type_traits>

///
/// \brief Version - Major.
///
#define NL_VERSION_MAJOR 0
///
/// \brief Version - Minor.
///
#define NL_VERSION_MINOR 14
/// \brief Version - Patch level.
///
#define NL_VERSION_PATCHLEVEL 0

///
/// \addtogroup compiledefine
/// \{

// -----------------------------------------------------------------------------
// General
// -----------------------------------------------------------------------------

/// \brief  Compile in academic solvers
///
/// Set to 0 to disable compiling the following solvers:
///
/// Sherman-Morrison, Woodbury, SOR and GMRES
///
/// In addition, compilation of FLOAT, LONGDOUBLE and FLOATQ128 matrix
/// solvers will be disabled.
/// GMRES may be added to productive solvers in the future
/// again. Compiling in all solvers may increase compile
/// time significantly.
///
#ifndef NL_USE_ACADEMIC_SOLVERS
	#define NL_USE_ACADEMIC_SOLVERS (1)
#endif

/// \brief Use backward Euler integration
///
/// This will use backward Euler instead of trapezoidal integration.
///
/// FIXME: Long term this will become a runtime setting. Only the capacitor
/// model currently has a trapezoidal version and there is no support currently
/// for variable capacitors. The change will have impact on timings since
/// trapezoidal improves timing accuracy.
#ifndef NL_USE_BACKWARD_EULER
	#define NL_USE_BACKWARD_EULER (1) // FIXME: Move to config struct later
#endif

/// \brief  Compile with core terminals owned by net_t
///
/// Set to 1 to enable that net_t owns core terminal collection.
/// In addition, this approach will use plib::list_t so that the doubled linked
/// list pointers are part of the core_terminal_t.
///
/// This approach requires that terminals at each point in time are only owned
/// by one net. The approach helped to identify bugs. Since the approach
/// involves that containers are located in different locations, it needs a
/// pre-processor define.
///
/// By default the setting is currently disabled.
///
#ifndef NL_USE_INPLACE_CORE_TERMS
	#define NL_USE_INPLACE_CORE_TERMS (0)
#endif

/// \brief Use alternative truth table execution approach
///
/// Enabling this will define a separate truth table execution approach which
/// will use a separate delegate for each input. This approach needs more
/// refinement yet. It works, but is not as fast as the standard approach.
///
/// Unfortunately this has to be macro since it needs another member variable.
///
#ifndef NL_USE_TT_ALTERNATIVE
	#define NL_USE_TT_ALTERNATIVE (0)
#endif

/// \brief  Compile matrix solvers using the __float128 type.
///
/// Defaults to \ref PUSE_FLOAT128
#ifndef NL_USE_FLOAT128
	#define NL_USE_FLOAT128 PUSE_FLOAT128
#endif

// -----------------------------------------------------------------------------
//  DEBUGGING
// -----------------------------------------------------------------------------

/// \brief Enable compile time debugging code
///

#ifndef NL_DEBUG
	#define NL_DEBUG (false)
#endif

///
/// \}
///

namespace netlist
{
	// -------------------------------------------------------------------------
	// GENERAL
	// -------------------------------------------------------------------------

	struct config_default
	{
		/// \brief  Make use of a memory pool for performance related objects.
		///
		/// Set to 1 to compile netlist with memory allocations from a
		/// linear memory pool. This is based of the assumption that
		/// due to enhanced locality there will be less cache misses.
		/// Your mileage may vary.
		///
		using use_mempool = std::integral_constant<bool, true>;

		/// brief default minimum alignment of mempool_arena
		///
		/// 256 is the best compromise between logic applications like MAME
		/// TTL games (e.g. pong) and analog applications like e.g. kidnikik
		/// sound.
		///
		/// Best performance for pong is achieved with a value of 16, but this
		/// degrades kidniki performance by ~10%.
		///
		/// More work is needed here.
		using mempool_align = std::integral_constant<std::size_t, 16>;

		/// \brief Prefer 128bit int type for ptime if supported
		///
		/// Set this to one if you want to use 128 bit int for ptime.
		/// This is about 10% slower on a skylake processor for pong.
		///
		using prefer_int128 = std::integral_constant<bool, false>;

		/// \brief  Enable queue statistics.
		///
		/// Queue statistics come at a performance cost. Although
		/// the cost is low, we disable them here since they are
		/// only needed during development.
		///
		using use_queue_stats = std::integral_constant<bool, false>;

		// ---------------------------------------------------------------------
		// Time resolution
		// ---------------------------------------------------------------------

		/// \brief Resolution as clocks per second for timing
		///
		/// Uses 100 picosecond resolution. This is aligned to MAME's
		/// `attotime` resolution.
		///
		/// The table below shows the maximum run times depending on
		/// time type size and resolution.
		///
		///  | Bits |               Res |       Seconds |   Days | Years |
		///  | ====-|               ===-|       =======-|   ====-| =====-|
		///  |  63  |     1,000,000,000 | 9,223,372,037 | 106,752| 292.3 |
		///  |  63  |    10,000,000,000 |   922,337,204 |  10,675|  29.2 |
		///  |  63  |   100,000,000,000 |    92,233,720 |   1,068|   2.9 |
		///  |  63  | 1,000,000,000,000 |     9,223,372 |     107|   0.3 |
		///
		using INTERNAL_RES = std::integral_constant<long long int,
			10'000'000'000LL>; // NOLINT

		/// \brief Recommended clock to be used
		///
		/// This is the recommended clock to be used in fixed clock applications
		/// limited to 32 bit clock resolution. The MAME code (netlist.cpp)
		/// contains code illustrating how to deal with remainders if \ref
		/// INTERNAL_RES is bigger than NETLIST_CLOCK.
		using DEFAULT_CLOCK = std::integral_constant<int,
			1'000'000'000>; // NOLINT

		/// \brief Default logic family
		///
		static constexpr const char *DEFAULT_LOGIC_FAMILY() { return "74XX"; }

		/// \brief Maximum queue size
		///
		using max_queue_size = std::integral_constant<std::size_t,
			1024>; // NOLINT

		/// \brief Maximum queue size for solvers
		///
		using max_solver_queue_size = std::integral_constant<std::size_t,
			512>; // NOLINT

		/// \brief Support float type for matrix calculations.
		///
		/// Defaults to NL_USE_ACADEMIC_SOLVERS to provide faster build times
		using use_float_matrix = std::integral_constant<bool,
			NL_USE_ACADEMIC_SOLVERS>;

		/// \brief Support long double type for matrix calculations.
		///
		/// Defaults to NL_USE_ACADEMIC_SOLVERS to provide faster build times
		using use_long_double_matrix = std::integral_constant<bool,
			NL_USE_ACADEMIC_SOLVERS>;

		using use_float128_matrix = std::integral_constant<bool,
			NL_USE_FLOAT128>;

		/// \brief  Floating point types used
		///
		/// nl_fptype is the floating point type used throughout the
		/// netlist core.
		///
		///  Don't change this! Simple analog circuits like pong
		///  work with float. Kidniki just doesn't work at all.
		///
		///  FIXME: More work needed. Review magic numbers.
		///
		using fptype = double;

		/// \brief  Store input values in logic_terminal_t.
		///
		/// Set to 1 to store values in logic_terminal_t instead of
		/// accessing them indirectly by pointer from logic_net_t.
		/// This approach is stricter and should identify bugs in
		/// the netlist core faster. The approach needs
		/// NL_USE_INPLACE_CORE_TERMS enabled to achieve some performance.
		///
		/// By default it is disabled since it is not as fast as
		/// the default approach. It is ~20% slower.
		///
		using use_copy_instead_of_reference = std::integral_constant<bool,
			false>;

		/// \brief Avoid unnecessary queue pushes
		///
		/// Enable the setting below to avoid queue pushes were at execution
		/// no action will be taken. This is academically cleaner, but
		/// ~6% slower than allowing this to happen and filter it during
		/// during "process".
		///
		using avoid_noop_queue_pushes = std::integral_constant<bool, false>;

		/// \brief Which sorted queue to use
		///
		/// Use timed_queue_heap to use stdc++ heap functions instead of the
		/// linear processing queue. This slows down execution by about 35%
		/// on a Kaby Lake.
		///
		/// The default is the  linear queue.

		// template <class A, class T>
		// using timed_queue = plib::timed_queue_heap<A, T>;

		template <typename A, typename T>
		using timed_queue = plib::timed_queue_linear<A, T>;
	};

	/// \brief  Netlist configuration.
	///
	/// You can override all netlist build defaults here which are defined
	/// in \ref config_default.
	///
	struct config : public config_default
	{
		// using mempool_align = std::integral_constant<std::size_t, 32>;
		// using avoid_noop_queue_pushes = std::integral_constant<bool, true>;
		// using use_copy_instead_of_reference = std::integral_constant<bool,
		// true>;
	};

	using nl_fptype = config::fptype;

	/// \brief  Specific constants depending on floating type
	///
	/// \tparam FT floating point type: double/float
	///
	template <typename FT>
	struct fp_constants
	{
	};

	/// \brief  Specific constants for long double floating point type
	///
	template <>
	struct fp_constants<long double>
	{
		static constexpr long double DIODE_MAXDIFF() noexcept
		{
			return 1e100L;
		} // NOLINT
		static constexpr long double DIODE_MAXVOLT() noexcept
		{
			return 300.0L;
		} // NOLINT

		static constexpr long double TIMESTEP_MAXDIFF() noexcept
		{
			return 1e100L;
		} // NOLINT
		static constexpr long double TIMESTEP_MINDIV() noexcept
		{
			return 1e-60L;
		} // NOLINT

		static constexpr const char *name() noexcept { return "long double"; }
		static constexpr const char *suffix() noexcept { return "L"; }
	};

	/// \brief  Specific constants for double floating point type
	///
	template <>
	struct fp_constants<double>
	{
		static constexpr double DIODE_MAXDIFF() noexcept
		{
			return 1e100;
		} // NOLINT
		static constexpr double DIODE_MAXVOLT() noexcept
		{
			return 300.0;
		} // NOLINT

		static constexpr double TIMESTEP_MAXDIFF() noexcept
		{
			return 1e100;
		} // NOLINT
		static constexpr double TIMESTEP_MINDIV() noexcept
		{
			return 1e-60;
		} // NOLINT

		static constexpr const char *name() noexcept { return "double"; }
		static constexpr const char *suffix() noexcept { return ""; }
	};

	/// \brief  Specific constants for float floating point type
	///
	template <>
	struct fp_constants<float>
	{
		static constexpr float DIODE_MAXDIFF() noexcept
		{
			return 1e20F;
		} // NOLINT
		static constexpr float DIODE_MAXVOLT() noexcept
		{
			return 90.0F;
		} // NOLINT

		static constexpr float TIMESTEP_MAXDIFF() noexcept
		{
			return 1e30F;
		} // NOLINT
		static constexpr float TIMESTEP_MINDIV() noexcept
		{
			return 1e-8F;
		} // NOLINT

		static constexpr const char *name() noexcept { return "float"; }
		static constexpr const char *suffix() noexcept { return "f"; }
	};

#if (NL_USE_FLOAT128)
	/// \brief  Specific constants for FLOAT128 floating point type
	///
	template <>
	struct fp_constants<FLOAT128>
	{
	#if 0
		// MAME compile doesn't support Q
		static constexpr FLOAT128 DIODE_MAXDIFF() noexcept { return  1e100Q; }
		static constexpr FLOAT128 DIODE_MAXVOLT() noexcept { return  300.0Q; }

		static constexpr FLOAT128 TIMESTEP_MAXDIFF() noexcept { return  1e100Q; }
		static constexpr FLOAT128 TIMESTEP_MINDIV() noexcept { return  1e-60Q; }

		static constexpr const char * name() noexcept { return "FLOAT128"; }
		static constexpr const char * suffix() noexcept { return "Q"; }
	#else
		static constexpr FLOAT128 DIODE_MAXDIFF() noexcept
		{
			return static_cast<FLOAT128>(1e100L);
		}
		static constexpr FLOAT128 DIODE_MAXVOLT() noexcept
		{
			return static_cast<FLOAT128>(300.0L);
		}

		static constexpr FLOAT128 TIMESTEP_MAXDIFF() noexcept
		{
			return static_cast<FLOAT128>(1e100L);
		}
		static constexpr FLOAT128 TIMESTEP_MINDIV() noexcept
		{
			return static_cast<FLOAT128>(1e-60L);
		}

		static constexpr const char *name() noexcept { return "__float128"; }
		static constexpr const char *suffix() noexcept { return "Q"; }

	#endif
	};
#endif
} // namespace netlist

//============================================================
//  Asserts
//============================================================

#define nl_assert(x)                                                           \
	do                                                                         \
	{                                                                          \
		if (NL_DEBUG)                                                          \
			passert_always(x);                                                 \
	} while (0)
#define nl_assert_always(x, msg) passert_always_msg(x, msg)

#endif // NLCONFIG_H_
