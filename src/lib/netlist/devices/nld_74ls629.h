// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74LS629.h
 *
 *  SN74LS629: VOLTAGE-CONTROLLED OSCILLATORS
 *
 *          +--------------+
 *      2FC |1     ++    16| VCC
 *      1FC |2           15| QSC VCC
 *     1RNG |3           14| 2RNG
 *     1CX1 |4  74LS629  13| 2CX1
 *     1CX2 |5           12| 2CX2
 *     1ENQ |6           11| 2ENQ
 *       1Y |7           10| 2Y
 *  OSC GND |8            9| GND
 *          +--------------+
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 *  NOTE: The CX1 and CX2 pins are not connected!
 *        The capacitor value has to be specified as a parameter.
 *        There are more comments on the challenges of emulating this
 *        chip in the *.c file
 *
 */

#ifndef NLD_74LS629_H_
#define NLD_74LS629_H_

#include "netlist/nl_setup.h"

#if 0
#define SN74LS629(name, p_cap)                                                  \
		NET_REGISTER_DEV(SN74LS629, name)                                       \
		NETDEV_PARAMI(name, CAP, p_cap)

#define SN74LS629_DIP(name, p_cap1, p_cap2)                                     \
		NET_REGISTER_DEV(SN74LS629_DIP, name)                                   \
		NETDEV_PARAMI(name, 1.CAP, p_cap1)                                      \
		NETDEV_PARAMI(name, 2.CAP, p_cap2)
#else
#define SN74LS629(name, p_cap)                                                  \
		NET_REGISTER_DEVEXT(SN74LS629, name, p_cap)

#define SN74LS629_DIP(name, p_cap1, p_cap2)                                     \
		NET_REGISTER_DEVEXT(SN74LS629_DIP, name, p_cap1, p_cap2)
#endif
#endif /* NLD_74LS629_H_ */
