// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLM_ROMS_H_
#define NLM_ROMS_H_

///
/// \file nlm_roms.h
///

#include "netlist/nl_setup.h"

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#if !NL_AUTO_DEVICES

#define PROM_82S126_DIP(name)                                                  \
		NET_REGISTER_DEV(PROM_82S126_DIP, name)

#define PROM_82S123_DIP(name)                                                  \
		NET_REGISTER_DEV(PROM_82S123_DIP, name)

#define PROM_74S287_DIP(name)                                                  \
		NET_REGISTER_DEV(PROM_74S287_DIP, name)

#define EPROM_2716_DIP(name)                                                   \
		NET_REGISTER_DEV(EPROM_2716_DIP, name)

#define TTL_82S16_DIP(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_82S16_DIP, __VA_ARGS__)

#define PROM_82S115_DIP(name)                                                  \
		NET_REGISTER_DEV(PROM_82S115_DIP, name)

#define RAM_2102A_DIP(name)                                                    \
		NET_REGISTER_DEV(RAM_2102A_DIP, name)

#endif // NL_AUTO_DEVICES


/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

// moved to net_lib.h

#endif // __PLIB_PREPROCESSOR__

#endif
