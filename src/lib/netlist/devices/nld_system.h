// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_system.h
 *
 * netlist devices defined in the core
 */

#ifndef NLD_SYSTEM_H_
#define NLD_SYSTEM_H_

#include "netlist/nl_setup.h"

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

#define TTL_INPUT(name, v)                                                      \
		NET_REGISTER_DEV(TTL_INPUT, name)                                       \
		PARAM(name.IN, v)

#define LOGIC_INPUT(name, v, family)                                            \
		NET_REGISTER_DEV(LOGIC_INPUT, name)                                     \
		PARAM(name.IN, v)                                                       \
		PARAM(name.FAMILY, family)

#define ANALOG_INPUT(name, v)                                                   \
		NET_REGISTER_DEV(ANALOG_INPUT, name)                                    \
		PARAM(name.IN, v)

#if 0
#define MAINCLOCK(name, freq)                                                   \
		NET_REGISTER_DEV(MAINCLOCK, name)                                       \
		PARAM(name.FREQ, freq)
#else
#define MAINCLOCK(name, freq)                                                   \
		NET_REGISTER_DEVEXT(MAINCLOCK, name, freq)
#endif

#define CLOCK(name, freq)                                                       \
		NET_REGISTER_DEV(CLOCK, name)                                           \
		PARAM(name.FREQ, freq)

#define VARCLOCK(name, func)                                                    \
		NET_REGISTER_DEV(VARCLOCK, name)                                        \
		PARAM(name.FUNC, func)

#define EXTCLOCK(name, freq, pattern)                                           \
		NET_REGISTER_DEV(EXTCLOCK, name)                                        \
		PARAM(name.FREQ, freq)                                                  \
		PARAM(name.PATTERN, pattern)

#define GNDA()                                                                  \
		NET_REGISTER_DEV(GNDA, GND)

#define DUMMY_INPUT(name)                                                       \
		NET_REGISTER_DEV(DUMMY_INPUT, name)

//FIXME: Usage discouraged, use OPTIMIZE_FRONTIER instead
#define FRONTIER_DEV(name, cIN, cG, cOUT)                                       \
		NET_REGISTER_DEV(FRONTIER_DEV, name)                                    \
		NET_C(cIN, name.I)                                                      \
		NET_C(cG,  name.G)                                                      \
		NET_C(cOUT, name.Q)

#define RES_SWITCH(name, cIN, cP1, cP2)                                         \
		NET_REGISTER_DEV(RES_SWITCH, name)                                      \
		NET_C(cIN, name.I)                                                      \
		NET_C(cP1, name.1)                                                      \
		NET_C(cP2, name.2)

/* Default device to hold netlist parameters */
#define PARAMETERS(name)                                                        \
		NET_REGISTER_DEV(PARAMETERS, name)

#define AFUNC(name, p_N, p_F)                                                   \
		NET_REGISTER_DEV(AFUNC, name)                                           \
		PARAM(name.N, p_N)                                                      \
		PARAM(name.FUNC, p_F)


#endif /* NLD_SYSTEM_H_ */
