// license:GPL-2.0+
// copyright-holders:Couriersud

///
/// \file nltypes.h
///

/// \note never change the name to nl_types.h. This creates a conflict
///       with nl_types.h file provided by libc++ (clang, macosx)
///

#ifndef NLTYPES_H_
#define NLTYPES_H_

#include "nl_config.h"
#include "plib/pchrono.h"
#include "plib/pdynlib.h"
#include "plib/pfmtlog.h"
#include "plib/pmempool.h"
#include "plib/pstate.h"
#include "plib/pstring.h"
#include "plib/ptime.h"
#include "plib/putil.h"

#include <unordered_map>

namespace netlist
{
	/// \brief Constants and const calculations for the library
	///
	template<typename T>
	struct nlconst_base : public plib::constants<T>
	{
		using BC = plib::constants<T>;

		static inline constexpr T np_VT(T n=BC::one(), T temp=BC::T0()) noexcept
		{ return n * temp * BC::k_b() / BC::Q_e(); }

		static inline constexpr T np_Is() noexcept { return static_cast<T>(1e-15); } // NOLINT

		static inline constexpr std::size_t max_queue_size() { return 512; } // NOLINT

		/// \brief constant startup gmin
		///
		/// This should be used during object creation to initialize
		/// conductivities with a reasonable value.
		/// During reset, the object should than make use of exec().gmin()
		/// to use the actual gmin() value.
		static inline constexpr T cgmin() noexcept { return BC::magic(1e-9); } // NOLINT

		// FIXME: Some objects used 1e-15 for initial gmin. Needs to be
		//        aligned with cgmin
		static inline constexpr T cgminalt() noexcept { return BC::magic(1e-15); } // NOLINT

		/// \brief Multiplier applied to VT in np diode models to determine range for constant model
		///
		static inline constexpr T diode_min_cutoff_mult() noexcept { return BC::magic(-5.0); } // NOLINT

		/// \brief Startup voltage used by np diode models
		///
		static inline constexpr T diode_start_voltage() noexcept { return BC::magic(0.7); } // NOLINT

	};

	/// \brief nlconst_base struct specialized for nl_fptype.
	///
	struct nlconst : public nlconst_base<nl_fptype>
	{
	};

	/// \brief netlist_sig_t is the type used for logic signals.
	///
	/// This may be any of bool, uint8_t, uint16_t, uin32_t and uint64_t.
	/// The choice has little to no impact on performance.
	///
	using netlist_sig_t = std::uint32_t;

	/// \brief Interface definition for netlist callbacks into calling code
	///
	/// A class inheriting from netlist_callbacks_t has to be passed to the netlist_t
	/// constructor. Netlist does processing during construction and thus needs
	/// the object passed completely constructed.
	///
	class callbacks_t
	{
	public:

		callbacks_t() = default;
		virtual ~callbacks_t() = default;

		PCOPYASSIGNMOVE(callbacks_t, default)

		/// \brief logging callback.
		///
		virtual void vlog(const plib::plog_level &l, const pstring &ls) const noexcept = 0;

		/// \brief provide library with static solver implementations.
		///
		/// By default no static solvers are provided since these are
		/// determined by the specific use case. It is up to the implementor
		/// of a callbacks_t implementation to optionally provide such a collection
		/// of symbols.
		///
		virtual plib::unique_ptr<plib::dynlib_base> static_solver_lib() const;
	};

	using log_type =  plib::plog_base<callbacks_t, NL_DEBUG>;

	//============================================================
	//  Types needed by various includes
	//============================================================

	/// \brief The memory pool for netlist objects
	///
	/// \note This is not the right location yet.
	///

#if (NL_USE_MEMPOOL)
	using nlmempool = plib::mempool;
#else
	using nlmempool = plib::aligned_arena;
#endif

	/// \brief Owned pointer type for pooled allocations.
	///
	template <typename T>
	using owned_pool_ptr = nlmempool::owned_pool_ptr<T>;

	/// \brief Unique pointer type for pooled allocations.
	///

	template <typename T>
	using unique_pool_ptr = nlmempool::unique_pool_ptr<T>;

	namespace detail {

		/// \brief Enum specifying the type of object
		///
		enum class terminal_type {
			TERMINAL = 0, ///< object is an analog terminal
			INPUT    = 1, ///< object is an input
			OUTPUT   = 2, ///< object is an output
		};

	} // namespace detail

#if (PHAS_INT128)
	//using netlist_time = plib::ptime<INT128, NETLIST_INTERNAL_RES>;
	using netlist_time = plib::ptime<std::int64_t, NETLIST_INTERNAL_RES>;
	using netlist_time_ext = plib::ptime<INT128, NETLIST_INTERNAL_RES>;
#else
	using netlist_time = plib::ptime<std::int64_t, NETLIST_INTERNAL_RES>;
	using netlist_time_ext = netlist_time;
#endif
	static_assert(noexcept(netlist_time::from_nsec(1)), "Not evaluated as constexpr");

	//============================================================
	//  MACROS
	//============================================================

	template <typename T> inline constexpr netlist_time NLTIME_FROM_NS(T &&t) noexcept { return netlist_time::from_nsec(t); }
	template <typename T> inline constexpr netlist_time NLTIME_FROM_US(T &&t) noexcept { return netlist_time::from_usec(t); }
	template <typename T> inline constexpr netlist_time NLTIME_FROM_MS(T &&t) noexcept { return netlist_time::from_msec(t); }

	struct desc_base
	{
		/// \brief: used to hold one static netlist_time value
		///
		template<netlist_time::internal_type value0>
		struct times_ns1
		{
			static constexpr netlist_time value(std::size_t N = 0)
			{
				plib::unused_var(N);
				return NLTIME_FROM_NS(value0);
			}
		};

		template <netlist_time::internal_type value0>
		using time_ns = times_ns1<value0>;

		/// \brief: used to hold two static netlist_time values
		///
		template<netlist_time::internal_type value0,
			netlist_time::internal_type  value1>
		struct times_ns2
		{
			static constexpr netlist_time value(std::size_t N)
			{
				return NLTIME_FROM_NS(N == 0 ? value0 : value1);
			}
		};

		/// \brief: used to hold two static netlist_time values
		///
		template<netlist_time::internal_type value0,
			netlist_time::internal_type value1,
			netlist_time::internal_type value2>
		struct times_ns3
		{
			static constexpr netlist_time value(std::size_t N)
			{
				return NLTIME_FROM_NS(N == 0 ? value0 :
						N == 1 ? value1 :
								 value2);
			}
		};

		/// \brief: used define a constant in device description struct
		///
		/// See the 74125 implementation
		///
		template <std::size_t V>
		using desc_const =  std::integral_constant<std::size_t, V>;
	};


} // namespace netlist


#endif // NLTYPES_H_
