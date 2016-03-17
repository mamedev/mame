// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    net_lib.h

    Discrete netlist implementation.

****************************************************************************/

#ifndef NET_LIB_H
#define NET_LIB_H

#include "nl_base.h"
#include "nld_signal.h"
#include "nld_system.h"

#include "nld_4020.h"
#include "nld_4066.h"
#include "nld_7402.h"
#include "nld_7404.h"
#include "nld_7408.h"
#include "nld_7410.h"
#include "nld_7411.h"
#include "nld_7420.h"
#include "nld_7425.h"
#include "nld_7427.h"
#include "nld_7430.h"
#include "nld_7432.h"
#include "nld_7437.h"
#include "nld_7448.h"
#include "nld_7450.h"
#include "nld_7474.h"
#include "nld_7483.h"
#include "nld_7486.h"
#include "nld_7490.h"
#include "nld_7493.h"
#include "nld_74107.h"
#include "nld_74123.h"
#include "nld_74153.h"
#include "nld_74175.h"
#include "nld_74192.h"
#include "nld_74193.h"
#include "nld_74279.h"
#include "nld_74ls629.h"
#include "nld_82S16.h"
#include "nld_9310.h"
#include "nld_9312.h"
#include "nld_9316.h"

#include "nld_ne555.h"
#include "nld_mm5837.h"

#include "nld_r2r_dac.h"

#include "nld_log.h"

#include "macro/nlm_cd4xxx.h"
#include "macro/nlm_ttl74xx.h"
#include "macro/nlm_opamp.h"
#include "macro/nlm_other.h"

#include "analog/nld_bjt.h"
#include "analog/nld_fourterm.h"
#include "analog/nld_switches.h"
#include "analog/nld_twoterm.h"
#include "analog/nld_opamps.h"
#include "solver/nld_solver.h"

#include "nld_legacy.h"

NETLIST_EXTERNAL(diode_models)
NETLIST_EXTERNAL(bjt_models)
NETLIST_EXTERNAL(family_models)

namespace netlist {
	void initialize_factory(netlist::factory_list_t &factory);
}

#endif
