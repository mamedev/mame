// license:GPL-2.0+
// copyright-holders:Ryan Holtz

#ifndef NLM_MC3340_H_
#define NLM_MC3340_H_

///
/// \file nlm_mc3340.h
///

#include "netlist/nl_setup.h"

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#if !NL_AUTO_DEVICES

#define MC3340_DIP(name)                                                       \
		NET_REGISTER_DEV(MC3340_DIP, name)

#endif // NL_AUTO_DEVICES

/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

// moved to net_lib.h

#endif // __PLIB_PREPROCESSOR__

#endif
