// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLBASE_H_
#define NLBASE_H_

///
/// \file nl_base.h
///

#if 1 || defined(NL_PROHIBIT_BASEH_INCLUDE)
//#error "nl_base.h included. Please correct."
#include "core/device.h"
#include "core/device_macros.h"
#include "core/devices.h"
#include "core/logic.h"
#include "core/object_array.h"

#else

#include "core/analog.h"
#include "core/base_objects.h"
#include "core/device.h"
#include "core/device_macros.h"
#include "core/devices.h"
#include "core/exec.h"
#include "core/logic.h"
#include "core/logic_family.h"
#include "core/netlist_state.h"
#include "core/nets.h"
#include "core/object_array.h"
#include "core/param.h"
#include "core/state_var.h"
#endif

//============================================================
// Namespace starts
//============================================================

namespace netlist
{

	// -----------------------------------------------------------------------------
	// Externals
	// -----------------------------------------------------------------------------

} // namespace netlist

#endif // NLBASE_H_
