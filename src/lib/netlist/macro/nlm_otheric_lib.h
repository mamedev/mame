// license:CC0
// copyright-holders:Couriersud

#ifndef NLM_OTHER_H_
#define NLM_OTHER_H_

///
/// \file nlm_other.h
///
///
#include "../nl_setup.h"

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#if !NL_AUTO_DEVICES

#define MC14584B_GATE(name)                                                    \
		NET_REGISTER_DEV(MC14584B_GATE, name)

#define MC14584B_DIP(name)                                                     \
		NET_REGISTER_DEV(MC14584B_DIP, name)

#define NE566_DIP(name)                                                        \
		NET_REGISTER_DEV(NE566_DIP, name)

// usage       : NE555_DIP(name)
#define NE555_DIP(...)                                                 \
	NET_REGISTER_DEVEXT(NE555_DIP, __VA_ARGS__)

// usage       : MC1455P_DIP(name)
#define MC1455P_DIP(...)                                               \
	NET_REGISTER_DEVEXT(MC1455P_DIP, __VA_ARGS__)

#endif

/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

// moved to net_lib.h

#endif

#endif
