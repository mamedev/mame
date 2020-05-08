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
		PARAM(name.MODEL, family)

#define LOGIC_INPUT8(name, v, family)                                           \
		NET_REGISTER_DEV(LOGIC_INPUT8, name)                                    \
		PARAM(name.IN, v)                                                       \
		PARAM(name.MODEL, family)

#define ANALOG_INPUT(name, v)                                                   \
		NET_REGISTER_DEV(ANALOG_INPUT, name)                                    \
		PARAM(name.IN, v)

#define MAINCLOCK(name, freq)                                                   \
		NET_REGISTER_DEVEXT(MAINCLOCK, name, freq)

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

#define NC_PIN(name)                                                            \
		NET_REGISTER_DEV(NC_PIN, name)

//FIXME: Usage discouraged, use OPTIMIZE_FRONTIER instead
#define FRONTIER_DEV(name, cIN, cG, cOUT)                                       \
		NET_REGISTER_DEV(FRONTIER_DEV, name)                                    \
		NET_C(cIN, name.I)                                                      \
		NET_C(cG,  name.G)                                                      \
		NET_C(cOUT, name.Q)

// FIXME ... remove parameters
#define SYS_DSW(name, pI, p1, p2)                                              \
        NET_REGISTER_DEVEXT(SYS_DSW, name, pI, p1, p2)

#define SYS_DSW2(name)                                                         \
        NET_REGISTER_DEV(SYS_DSW2, name)

#define SYS_COMPD(name)                                                        \
        NET_REGISTER_DEV(SYS_COMPD, name)

#define SYS_NOISE_MT_U(name, pSIGMA)                                           \
        NET_REGISTER_DEVEXT(SYS_NOISE_MT_U, name, pSIGMA)

#define SYS_NOISE_MT_N(name, pSIGMA)                                           \
        NET_REGISTER_DEVEXT(SYS_NOISE_MT_N, name, pSIGMA)

/* Default device to hold netlist parameters */
#define PARAMETERS(name)                                                        \
		NET_REGISTER_DEV(PARAMETERS, name)

#define AFUNC(name, p_N, p_F)                                                   \
		NET_REGISTER_DEV(AFUNC, name)                                           \
		PARAM(name.N, p_N)                                                      \
		PARAM(name.FUNC, p_F)


#endif /* NLD_SYSTEM_H_ */
