// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 * nld_4136.h
 *
 *  CD4066: Quad Analog Switch with Level Translation
 *
 *          +--------------+
 *       1Z |1     ++    16| VCC
 *       1Y |2           15| 1S
 *       2Y |3           14| 4S
 *       2Z |4    4066   13| 4Z
 *       2S |5           12| 4Y
 *       3S |6           11| 3Y
 *       /E |7           10| 3Z
 *      GND |8            9| VEE
 *          +--------------+
 *
 *  FIXME: These devices are slow (can be over 200 ns in HC types). This is currently not reflected
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 */

#ifndef NLD_4316_H_
#define NLD_4316_H_

#include "netlist/nl_setup.h"

#define CD4316_GATE(name)                                                       \
		NET_REGISTER_DEV(CD4316_GATE, name)

#endif /* NLD_4316_H_ */
