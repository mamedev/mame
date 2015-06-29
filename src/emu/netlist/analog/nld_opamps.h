// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_opamps.h
 *
 */

#pragma once

#ifndef NLD_OPAMPS_H_
#define NLD_OPAMPS_H_

#include "../nl_base.h"
#include "../nl_setup.h"
#include "nld_twoterm.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define LM3900(_name)                                                          \
	SUBMODEL(opamp_lm3900, name)

// ----------------------------------------------------------------------------------------
// Devices ...
// ----------------------------------------------------------------------------------------

NETLIST_EXTERNAL(opamp_lm3900);



#endif /* NLD_OPAMPS_H_ */
