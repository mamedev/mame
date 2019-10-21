// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_bjt.h
 *
 */

#ifndef NLD_BJT_H_
#define NLD_BJT_H_

#include "netlist/nl_setup.h"

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

#define QBJT_SW(name, model)                                                 \
		NET_REGISTER_DEV(QBJT_SW, name)                                       \
		NETDEV_PARAMI(name,  MODEL, model)

#define QBJT_EB(name, model)                                                 \
		NET_REGISTER_DEV(QBJT_EB, name)                                       \
		NETDEV_PARAMI(name,  MODEL, model)

#endif /* NLD_BJT_H_ */
