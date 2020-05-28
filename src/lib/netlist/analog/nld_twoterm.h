// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_TWOTERM_H_
#define NLD_TWOTERM_H_

///
/// \file nld_twoterm.h
///

#include "netlist/nl_setup.h"

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

#define RES(name, p_R)                                                         \
		NET_REGISTER_DEVEXT(RES, name, p_R)

#define POT(name, p_R)                                                         \
		NET_REGISTER_DEVEXT(POT, name, p_R)

// Does not have pin 3 connected
#define POT2(name, p_R)                                                        \
		NET_REGISTER_DEVEXT(POT2, name, p_R)

#define CAP(name, p_C)                                                         \
		NET_REGISTER_DEVEXT(CAP, name, p_C)

#define IND(name, p_L)                                                         \
		NET_REGISTER_DEVEXT(IND, name, p_L)

// Generic Diode
#define DIODE(name,  model)                                                    \
		NET_REGISTER_DEVEXT(DIODE, name, model)

// Generic Zener Diode
#define ZDIODE(name,  model)                                                   \
		NET_REGISTER_DEVEXT(ZDIODE, name, model)

#define VS(name, pV)                                                           \
		NET_REGISTER_DEVEXT(VS, name, pV)

#define CS(name, pI)                                                           \
		NET_REGISTER_DEVEXT(CS, name, pI)

#endif // NLD_TWOTERM_H_
