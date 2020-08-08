// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_4020_H_
#define NLD_4020_H_

#include "netlist/nl_setup.h"

/* FIXME: only used in mario.c */
#define CD4020_WI(name, cIP, cRESET, cVDD, cVSS)                                \
		NET_REGISTER_DEV(CD4020_WI, name)                                       \
		NET_CONNECT(name, IP, cIP)                                              \
		NET_CONNECT(name, RESET,  cRESET)                                       \
		NET_CONNECT(name, VDD,  cVDD)                                           \
		NET_CONNECT(name, VSS,  cVSS)

#define CD4020(name)                                                            \
		NET_REGISTER_DEV(CD4020, name)

#define CD4024(name)                                                            \
		NET_REGISTER_DEV(CD4024, name)

#endif /* NLD_4020_H_ */
