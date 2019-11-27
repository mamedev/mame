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
#include "plib/pfmtlog.h"
#include "plib/pmempool.h"
#include "plib/pstate.h"
#include "plib/pstring.h"
#include "plib/ptime.h"
#include "plib/putil.h"

#include <unordered_map>

namespace netlist
{
	/// \brief plib::constants struct specialized for nl_fptype.
	///
	struct nlconst : public plib::constants<nl_fptype>
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

		COPYASSIGNMOVE(callbacks_t, default)

		/// \brief logging callback.
		///
		virtual void vlog(const plib::plog_level &l, const pstring &ls) const noexcept = 0;

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
	using netlist_time = ptime<INT128, NETLIST_INTERNAL_RES>;
#else
	using netlist_time = plib::ptime<std::int64_t, NETLIST_INTERNAL_RES>;
	static_assert(noexcept(netlist_time::from_nsec(1)) == true, "Not evaluated as constexpr");
#endif

	//============================================================
	//  MACROS
	//============================================================

	template <typename T> inline constexpr const netlist_time NLTIME_FROM_NS(T &&t) noexcept { return netlist_time::from_nsec(t); }
	template <typename T> inline constexpr const netlist_time NLTIME_FROM_US(T &&t) noexcept { return netlist_time::from_usec(t); }
	template <typename T> inline constexpr const netlist_time NLTIME_FROM_MS(T &&t) noexcept { return netlist_time::from_msec(t); }

} // namespace netlist

namespace plib {

	template<>
	inline void state_manager_t::save_item(const void *owner, netlist::netlist_time &nlt, const pstring &stname)
	{
		save_state_ptr(owner, stname, datatype_t(sizeof(netlist::netlist_time::internal_type), true, false), 1, nlt.get_internaltype_ptr());
	}
} // namespace plib

#endif // NLTYPES_H_
