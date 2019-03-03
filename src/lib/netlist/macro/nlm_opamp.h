// license:GPL-2.0+
// copyright-holders:Couriersud
#ifndef NLM_OPAMP_H_
#define NLM_OPAMP_H_

#include "netlist/nl_setup.h"

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#ifndef NL_AUTO_DEVICES

#define MB3614_DIP(name)                                                      \
		NET_REGISTER_DEV(MB3614_DIP, name)

#define LM324_DIP(name)                                                       \
		NET_REGISTER_DEV(LM324_DIP, name)

#define LM358_DIP(name)                                                       \
		NET_REGISTER_DEV(LM358_DIP, name)

#define LM3900(name)                                                           \
		NET_REGISTER_DEV(LM3900, name)

#endif

/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

NETLIST_EXTERNAL(OPAMP_lib)

#endif

#endif
