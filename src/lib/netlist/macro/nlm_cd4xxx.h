// license:GPL-2.0+
// copyright-holders:Couriersud
#ifndef NLD_CD4XXX_H_
#define NLD_CD4XXX_H_

/// \file nlm_cd4xxx.h
///

#include "netlist/nl_setup.h"

/*
 * Devices:
 *
 * CD4001_NOR : single gate
 * CD4001_DIP : dip package
 * CD4013_DIP : dip package (device model in core)
 * CD4020_DIP : dip package (device model in core)
 * CD4024_DIP : dip package (device model in core)
 * CD4016_DIP : dip package (device model in core)
 * CD4053_DIP : dip package (device model in core)
 * CD4066_DIP : dip package (device model in core)
 *
 */

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#if !NL_AUTO_DEVICES

#define CD4001_GATE(name)                                                      \
		NET_REGISTER_DEV(CD4001_GATE, name)

#define CD4001_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4001_DIP, name)

#define CD4011_GATE(name)                                                      \
		NET_REGISTER_DEV(CD4011_GATE, name)

#define CD4011_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4011_DIP, name)

#define CD4069_GATE(name)                                                      \
		NET_REGISTER_DEV(CD4069_GATE, name)

#define CD4069_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4069_DIP, name)

#define CD4070_GATE(name)                                                      \
		NET_REGISTER_DEV(CD4070_GATE, name)

#define CD4070_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4070_DIP, name)

/* ----------------------------------------------------------------------------
 *  DIP only macros
 * ---------------------------------------------------------------------------*/

#define CD4013_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4013_DIP, name)

#define CD4020_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4020_DIP, name)

#define CD4024_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4024_DIP, name)

#define CD4053_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4053_DIP, name)

#define CD4066_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4066_DIP, name)

#define CD4016_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4016_DIP, name)

#define CD4316_DIP(name)                                                      \
		NET_REGISTER_DEV(CD4016_DIP, name)

#define CD4538_DIP(name)                                                        \
		NET_REGISTER_DEV(CD4538_DIP, name)

#endif // !NL_AUTO_DEVICES

/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

// moved to net_lib.h

#endif

#endif // NLD_CD4XXX_H_
