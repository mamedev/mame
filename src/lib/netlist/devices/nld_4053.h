// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4053.h
 *
 *  CD4053: Triple 2-Channel Analog Multiplexer/Demultiplexer
 *
 *          +--------------+
 *  INOUTBY |1     ++    16| VDD
 *  INOUTBX |2           15| OUTINB
 *  INOUTCY |3           14| OUTINA
 *   OUTINC |4    4053   13| INOUTAY
 *  INOUTCX |5           12| INOUTAX
 *      INH |6           11| A
 *      VEE |7           10| B
 *      VSS |8            9| C
 *          +--------------+
 *
 *  FIXME: These devices are slow (~125 ns). This is currently not reflected
 *
 *  Naming conventions follow National semiconductor datasheet
 *
 */

#ifndef NLD_4053_H_
#define NLD_4053_H_

#include "netlist/nl_setup.h"

// FIXME: Implement pure CMOS version

#define CD4053_GATE(name)                                                       \
		NET_REGISTER_DEV(CD4053_GATE, name)

#endif /* NLD_4053_H_ */
