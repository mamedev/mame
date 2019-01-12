// license:GPL-2.0+
// copyright-holders:Couriersud
/*!
 *
 * \file netlist_types.h
 *
 */

#ifndef NETLIST_TYPES_H_
#define NETLIST_TYPES_H_

#include "nl_config.h"
#include "plib/pchrono.h"
#include "plib/pstring.h"

#include <cstdint>
#include <unordered_map>

namespace netlist
{
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

	}
}

#endif /* NETLIST_TYPES_H_ */
