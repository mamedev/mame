// license:GPL-2.0+
// copyright-holders:Couriersud
/*!
 *
 * \file nl_types.h
 *
 */

#ifndef NLTYPES_H_
#define NLTYPES_H_

#include <cstdint>
#include <unordered_map>

#include "nl_config.h"
#include "plib/pchrono.h"
#include "plib/pstring.h"

namespace netlist
{
	//============================================================
	//  Performance tracking
	//============================================================

	#if NL_KEEP_STATISTICS
	using nperftime_t = plib::chrono::timer<plib::chrono::exact_ticks, true>;
	using nperfcount_t = plib::chrono::counter<true>;
	#else
	using nperftime_t = plib::chrono::timer<plib::chrono::exact_ticks, false>;
	using nperfcount_t = plib::chrono::counter<false>;
	#endif

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

#endif /* NLTYPES_H_ */
