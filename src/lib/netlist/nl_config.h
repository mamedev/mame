// license:GPL-2.0+
// copyright-holders:Couriersud

/// \file nl_config.h
///

#ifndef NLCONFIG_H_
#define NLCONFIG_H_

#include "plib/pconfig.h"
#include "plib/pexception.h"

/// \addtogroup compiledefine
/// \{

//============================================================
//  GENERAL
//============================================================

/// \brief  Make use of a memory pool for performance related objects.
///
/// Set to 1 to compile netlist with memory allocations from a
/// linear memory pool. This is based of the assumption that
/// due to enhanced locality there will be less cache misses.
/// Your mileage may vary.
///

#ifndef NL_USE_MEMPOOL
#define NL_USE_MEMPOOL               (1)
#endif

/// \brief  Enable queue statistics.
///
/// Queue statistics come at a performance cost. Although
/// the cost is low, we disable them here since they are
/// only needed during development.
///

#ifndef NL_USE_QUEUE_STATS
#define NL_USE_QUEUE_STATS             (0)
#endif

/// \brief  Store input values in logic_terminal_t.
///
/// Set to 1 to store values in logic_terminal_t instead of
/// accessing them indirectly by pointer from logic_net_t.
/// This approach is stricter and should identify bugs in
/// the netlist core faster.
/// By default it is disabled since it is not as fast as
/// the default approach. It is up to 10% slower.
///

#ifndef NL_USE_COPY_INSTEAD_OF_REFERENCE
#define NL_USE_COPY_INSTEAD_OF_REFERENCE (0)
#endif

/// \brief  Use the truthtable implementation of 7448 instead of the coded device
///
/// FIXME: Using truthtable is a lot slower than the explicit device
///        in breakout. Performance drops by 20%. This can be fixed by
///        setting param USE_DEACTIVATE for the device.

#ifndef NL_USE_TRUTHTABLE_7448
#define NL_USE_TRUTHTABLE_7448 (0)
#endif

/// \brief  Use the truthtable implementation of 74107 instead of the coded device
///
/// FIXME: The truthtable implementation of 74107 (JK-Flipflop)
///        is included for educational purposes to demonstrate how
///        to implement state holding devices as truthtables.
///        It will completely nuke performance for pong.

#ifndef NL_USE_TRUTHTABLE_74107
#define NL_USE_TRUTHTABLE_74107 (0)
#endif

/// \brief  Use the __float128 type for matrix calculations.
///
/// Defaults to \ref PUSE_FLOAT128

#ifndef NL_USE_FLOAT128
#define NL_USE_FLOAT128 PUSE_FLOAT128
#endif

/// \brief Support float type for matrix calculations.
///
/// Defaults to off to provide faster build times

#ifndef NL_USE_FLOAT_MATRIX
#define NL_USE_FLOAT_MATRIX (0)
#endif

/// \brief Support long double type for matrix calculations.
///
/// Defaults to off to provide faster build times

#ifndef NL_USE_LONG_DOUBLE_MATRIX
#define NL_USE_LONG_DOUBLE_MATRIX (0)
#endif

//============================================================
//  DEBUGGING
//============================================================

/// \brief Enable compile time debugging code
///

#ifndef NL_DEBUG
#define NL_DEBUG                    (false)
//#define NL_DEBUG                    (true)
#endif

//============================================================
// Time resolution
//============================================================

/// \brief Resolution as clocks per second for timing
///
///  Uses nano-second resolution - Sufficient for now

static constexpr const auto NETLIST_INTERNAL_RES = 1000000000;

// FIXME: Belongs into MAME netlist.h
static constexpr const auto NETLIST_CLOCK = NETLIST_INTERNAL_RES;

///  \}

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
///

using nl_fptype = double;

namespace netlist
{
	/// \brief  Specific constants depending on floating type
	///
	/// \tparam FT floating point type: double/float
	///
	template <typename FT>
	struct fp_constants
	{ };

	/// \brief  Specific constants for long double floating point type
	///
	template <>
	struct fp_constants<long double>
	{
		static inline constexpr long double DIODE_MAXDIFF() noexcept { return  1e100L; }
		static inline constexpr long double DIODE_MAXVOLT() noexcept { return  300.0L; }

		static inline constexpr long double TIMESTEP_MAXDIFF() noexcept { return  1e100L; }
		static inline constexpr long double TIMESTEP_MINDIV() noexcept { return  1e-60L; }

		static inline constexpr const char * name() noexcept { return "long double"; }
		static inline constexpr const char * suffix() noexcept { return "L"; }
	};

	/// \brief  Specific constants for double floating point type
	///
	template <>
	struct fp_constants<double>
	{
		static inline constexpr double DIODE_MAXDIFF() noexcept { return  1e100; }
		static inline constexpr double DIODE_MAXVOLT() noexcept { return  300.0; }

		static inline constexpr double TIMESTEP_MAXDIFF() noexcept { return  1e100; }
		static inline constexpr double TIMESTEP_MINDIV() noexcept { return  1e-60; }

		static inline constexpr const char * name() noexcept { return "double"; }
		static inline constexpr const char * suffix() noexcept { return ""; }
	};

	/// \brief  Specific constants for float floating point type
	///
	template <>
	struct fp_constants<float>
	{
		static inline constexpr float DIODE_MAXDIFF() noexcept { return  1e20f; }
		static inline constexpr float DIODE_MAXVOLT() noexcept { return  90.0f; }

		static inline constexpr float TIMESTEP_MAXDIFF() noexcept { return  1e30f; }
		static inline constexpr float TIMESTEP_MINDIV() noexcept { return  1e-8f; }

		static inline constexpr const char * name() noexcept { return "float"; }
		static inline constexpr const char * suffix() noexcept { return "f"; }
	};

#if (NL_USE_FLOAT128)
	/// \brief  Specific constants for __float128 floating point type
	///
	template <>
	struct fp_constants<__float128>
	{
#if 0
		// MAME compile doesn't support Q
		static inline constexpr __float128 DIODE_MAXDIFF() noexcept { return  1e100Q; }
		static inline constexpr __float128 DIODE_MAXVOLT() noexcept { return  300.0Q; }

		static inline constexpr __float128 TIMESTEP_MAXDIFF() noexcept { return  1e100Q; }
		static inline constexpr __float128 TIMESTEP_MINDIV() noexcept { return  1e-60Q; }

		static inline constexpr const char * name() noexcept { return "__float128"; }
		static inline constexpr const char * suffix() noexcept { return "Q"; }
#else
	static inline constexpr __float128 DIODE_MAXDIFF() noexcept { return static_cast<__float128>(1e100L); }
	static inline constexpr __float128 DIODE_MAXVOLT() noexcept { return static_cast<__float128>(300.0L); }

	static inline constexpr __float128 TIMESTEP_MAXDIFF() noexcept { return static_cast<__float128>(1e100L); }
	static inline constexpr __float128 TIMESTEP_MINDIV() noexcept { return static_cast<__float128>(1e-60L); }

	static inline constexpr const char * name() noexcept { return "__float128"; }
	static inline constexpr const char * suffix() noexcept { return "Q"; }

#endif
	};
#endif
} // namespace netlist

//============================================================
//  Asserts
//============================================================

#if defined(MAME_DEBUG) || (NL_DEBUG == true)
#define nl_assert(x)    passert_always(x)
#else
#define nl_assert(x)    do { } while (0)
#endif
#define nl_assert_always(x, msg) passert_always_msg(x, msg)

#endif // NLCONFIG_H_
