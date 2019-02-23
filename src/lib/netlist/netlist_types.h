// license:GPL-2.0+
// copyright-holders:Couriersud
/*!
 *
 * \file netlist_types.h
 *
 */

#ifndef NETLIST_TYPES_H_
#define NETLIST_TYPES_H_

#include <cstdint>
#include <unordered_map>

#include "nl_config.h"
#include "plib/pchrono.h"
#include "plib/pfmtlog.h"
#include "plib/pmempool.h"
#include "plib/pstring.h"


namespace netlist
{
	/*! @brief netlist_sig_t is the type used for logic signals.
	 *
	 *  This may be any of bool, uint8_t, uint16_t, uin32_t and uint64_t.
	 *  The choice has little to no impact on performance.
	 */
	using netlist_sig_t = std::uint32_t;

	/* FIXME: belongs into nl_base.h to nlstate */
	/**
	 * @brief Interface definition for netlist callbacks into calling code
	 *
	 * A class inheriting from netlist_callbacks_t has to be passed to the netlist_t
	 * constructor. Netlist does processing during construction and thus needs
	 * the object passed completely constructed.
	 *
	 */
	class callbacks_t
	{
	public:

		callbacks_t() = default;
		/* what is done before this is passed as a unique_ptr to netlist
		 * we should not limit.
		 */
		virtual ~callbacks_t() = default;
		COPYASSIGNMOVE(callbacks_t, default)

		/* logging callback */
		virtual void vlog(const plib::plog_level &l, const pstring &ls) const = 0;

	};

	using log_type =  plib::plog_base<callbacks_t, NL_DEBUG>;


	//============================================================
	//  Performance tracking
	//============================================================

	template<bool enabled_>
	using nperftime_t = plib::chrono::timer<plib::chrono::exact_ticks, enabled_>;

	template<bool enabled_>
	using nperfcount_t = plib::chrono::counter<enabled_>;

	//============================================================
	//  Types needed by various includes
	//============================================================

	/*! The memory pool for netlist objects
	 *
	 * \note This is not the right location yet.
	 *
	 */

#if (USE_MEMPOOL)
	using nlmempool = plib::mempool;
#else
	using nlmempool = plib::mempool_default;
#endif

	/*! Owned pointer type for pooled allocations.
	 *
	 */
	template <typename T>
	using poolptr = nlmempool::poolptr<T>;

	inline nlmempool &pool()
	{
		static nlmempool static_pool(655360, 16);
		return static_pool;
	}

	namespace detail {

		/*! Enum specifying the type of object */
		enum terminal_type {
			TERMINAL = 0, /*!< object is an analog terminal */
			INPUT    = 1, /*!< object is an input */
			OUTPUT   = 2, /*!< object is an output */
		};

		/*! Type of the model map used.
		 *  This is used to hold all #Models in an unordered map
		 */
		using model_map_t = std::unordered_map<pstring, pstring>;

	} // namespace detail
} // namespace netlist

#endif /* NETLIST_TYPES_H_ */
