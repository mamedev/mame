// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4066.h
 *
 *  CD4066: Quad Bilateral Switch
 *
 *          +--------------+
 *   INOUTA |1     ++    14| VDD
 *   OUTINA |2           13| CONTROLA
 *   OUTINB |3           12| CONTROLD
 *   INOUTB |4    4066   11| INOUTD
 * CONTROLB |5           10| OUTIND
 * CONTROLC |6            9| OUTINC
 *      VSS |7            8| INOUTC
 *          +--------------+
 *
 *  FIXME: These devices are slow (~125 ns). This is currently not reflected
 *
 *  Naming conventions follow National semiconductor datasheet
 *
 */

#ifndef NLD_4066_H_
#define NLD_4066_H_

#include "netlist/nl_setup.h"

#define CD4066_GATE(name)                                                       \
		NET_REGISTER_DEV(CD4066_GATE, name)

#endif /* NLD_4066_H_ */
