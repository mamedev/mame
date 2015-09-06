#ifndef NLD_CD4XXX_H_
#define NLD_CD4XXX_H_

#include "nl_setup.h"

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

#define CD4001_NOR(_name)                                                      \
		NET_REGISTER_DEV(CD4001_NOR, _name)

#define CD4001_DIP(_name)                                                      \
		NET_REGISTER_DEV(CD4001_DIP, _name)

/* ----------------------------------------------------------------------------
 *  DIP only macros
 * ---------------------------------------------------------------------------*/

#define CD4020_DIP(_name)                                                      \
		NET_REGISTER_DEV(CD4020_DIP, _name)

#define CD4066_DIP(_name)                                                      \
		NET_REGISTER_DEV(CD4066_DIP, _name)

#define CD4016_DIP(_name)                                                      \
		NET_REGISTER_DEV(CD4016_DIP, _name)

/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

NETLIST_EXTERNAL(CD4XXX_lib)

#endif

#endif
