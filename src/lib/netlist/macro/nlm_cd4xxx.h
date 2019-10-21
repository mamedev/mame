// license:GPL-2.0+
// copyright-holders:Couriersud
#ifndef NLD_CD4XXX_H_
#define NLD_CD4XXX_H_

#include "netlist/nl_setup.h"

/*
 * Devices:
 *
 * CD4001_NOR : single gate
 * CD4001_DIP : dip package
 * CD4020_DIP : dip package (device model in core)
 * CD4016_DIP : dip package (device model in core)
 * CD4066_DIP : dip package (device model in core)
 *
 */

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#ifndef NL_AUTO_DEVICES

#define CD4001_NOR(name)                                                      \
		NET_REGISTER_DEV(CD4001_NOR, name)

#define CD4001_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4001_DIP, name)

/* ----------------------------------------------------------------------------
 *  DIP only macros
 * ---------------------------------------------------------------------------*/

#define CD4020_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4020_DIP, name)

#define CD4066_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4066_DIP, name)

#define CD4016_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4016_DIP, name)

#define CD4316_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4016_DIP, name)

#endif
/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

NETLIST_EXTERNAL(CD4XXX_lib)

#endif

#endif
