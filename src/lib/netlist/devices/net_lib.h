// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NET_LIB_H
#define NET_LIB_H

///
/// \file net_lib.h
///
/// Discrete netlist implementation.
///

#include "netlist/nl_setup.h"

//#define NL_AUTO_DEVICES 1

#ifdef NL_AUTO_DEVICES
#include "nld_devinc.h"

// FIXME: copied from nld_twoterm.h
#ifdef RES_R
#warning "Do not include rescap.h in a netlist environment"
#endif
#ifndef RES_R
#define RES_R(res) (res)
#define RES_K(res) ((res) * 1e3)
#define RES_M(res) ((res) * 1e6)
#define CAP_U(cap) ((cap) * 1e-6)
#define CAP_N(cap) ((cap) * 1e-9)
#define CAP_P(cap) ((cap) * 1e-12)
#define IND_U(ind) ((ind) * 1e-6)
#define IND_N(ind) ((ind) * 1e-9)
#define IND_P(ind) ((ind) * 1e-12)
#endif

#include "netlist/macro/nlm_cd4xxx.h"
#include "netlist/macro/nlm_opamp.h"
#include "netlist/macro/nlm_other.h"
#include "netlist/macro/nlm_ttl74xx.h"

#include "nld_7448.h"

#else

#define SOLVER(name, freq)                                                  \
		NET_REGISTER_DEV(SOLVER, name)                                      \
		PARAM(name.FREQ, freq)

#include "nld_system.h"

#include "nld_2102A.h"
#include "nld_2716.h"
#include "nld_4020.h"
#include "nld_4066.h"
#include "nld_74107.h"
#include "nld_74123.h"
#include "nld_74153.h"
#include "nld_74161.h"
#include "nld_74164.h"
#include "nld_74165.h"
#include "nld_74166.h"
#include "nld_74174.h"
#include "nld_74175.h"
#include "nld_74192.h"
#include "nld_74193.h"
#include "nld_74194.h"
#include "nld_74365.h"
#include "nld_7448.h"
#include "nld_7450.h"
#include "nld_7473.h"
#include "nld_7474.h"
#include "nld_7475.h"
#include "nld_7483.h"
#include "nld_7485.h"
#include "nld_7490.h"
#include "nld_7493.h"
#include "nld_7497.h"
#include "nld_74ls629.h"
#include "nld_82S115.h"
#include "nld_82S123.h"
#include "nld_82S126.h"
#include "nld_82S16.h"
#include "nld_9310.h"
#include "nld_9316.h"
#include "nld_9322.h"
#include "nld_tms4800.h"

#include "nld_am2847.h"
#include "nld_dm9314.h"
#include "nld_dm9334.h"

#include "nld_mm5837.h"
#include "nld_ne555.h"

#include "nld_r2r_dac.h"

#include "nld_schmitt.h"

#include "nld_tristate.h"

#include "nld_log.h"

#include "netlist/macro/nlm_cd4xxx.h"
#include "netlist/macro/nlm_opamp.h"
#include "netlist/macro/nlm_other.h"
#include "netlist/macro/nlm_ttl74xx.h"

#include "netlist/analog/nld_bjt.h"
#include "netlist/analog/nld_fourterm.h"
#include "netlist/analog/nld_mosfet.h"
#include "netlist/analog/nld_opamps.h"
#include "netlist/analog/nld_switches.h"
#include "netlist/analog/nld_twoterm.h"

#include "nld_legacy.h"
#endif

#endif
