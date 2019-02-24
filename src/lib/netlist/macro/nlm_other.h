// license:GPL-2.0+
// copyright-holders:Couriersud
#ifndef NLM_OTHER_H_
#define NLM_OTHER_H_

#include "netlist/nl_setup.h"

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#ifndef NL_AUTO_DEVICES

#define MC14584B_GATE(name)                                                   \
		NET_REGISTER_DEV(MC14584B_GATE, name)

#define MC14584B_DIP(name)                                                    \
		NET_REGISTER_DEV(MC14584B_DIP, name)

#endif

/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

NETLIST_EXTERNAL(otheric_lib)

#endif

#endif
