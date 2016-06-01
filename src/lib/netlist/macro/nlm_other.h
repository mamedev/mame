// license:GPL-2.0+
// copyright-holders:Couriersud
#ifndef NLM_OTHER_H_
#define NLM_OTHER_H_

#include "nl_setup.h"

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#define MC14584B_GATE(name)                                                   \
		NET_REGISTER_DEV(MC14584B_GATE, name)

#define MC14584B_DIP(name)                                                    \
		NET_REGISTER_DEV(MC14584B_DIP, name)


/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

NETLIST_EXTERNAL(otheric_lib)

#endif

#endif
