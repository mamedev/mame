// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    net_lib.h

    Discrete netlist implementation.

****************************************************************************/

#ifndef NET_LIB_H
#define NET_LIB_H

#include "netlist/nl_setup.h"

//#define NL_AUTO_DEVICES 1

#define SOLVER(name, freq)                                                  \
		NET_REGISTER_DEV(SOLVER, name)                                      \
		PARAM(name.FREQ, freq)

#ifdef NL_AUTO_DEVICES
#include "nld_devinc.h"

#include "macro/nlm_cd4xxx.h"
#include "macro/nlm_ttl74xx.h"
#include "macro/nlm_opamp.h"
#include "macro/nlm_other.h"

#include "nld_7448.h"

#else

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
#include "netlist/analog/nld_opamps.h"
#include "netlist/analog/nld_switches.h"
#include "netlist/analog/nld_twoterm.h"

#include "nld_legacy.h"
#endif

#endif
