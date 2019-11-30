// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_R2R_DAC.h
 *
 *  DMR2R_DAC: R-2R DAC
 *
 *  Generic R-2R DAC ... This is fast.
 *                 2R
 *  Bit n    >----RRR----+---------> Vout
 *                       |
 *                       R
 *                       R R
 *                       R
 *                       |
 *                       .
 *                       .
 *                 2R    |
 *  Bit 2    >----RRR----+
 *                       |
 *                       R
 *                       R R
 *                       R
 *                       |
 *                 2R    |
 *  Bit 1    >----RRR----+
 *                       |
 *                       R
 *                       R 2R
 *                       R
 *                       |
 *                      V0
 *
 * Using Thevenin's Theorem, this can be written as
 *
 *          +---RRR-----------> Vout
 *          |
 *          V
 *          V  V = VAL / 2^n * Vin
 *          V
 *          |
 *          V0
 *
 */

#ifndef NLD_R2R_DAC_H_
#define NLD_R2R_DAC_H_

#include "netlist/nl_setup.h"

#define R2R_DAC(name, p_VIN, p_R, p_N)                                          \
		NET_REGISTER_DEV(R2R_DAC, name)                                         \
		NETDEV_PARAMI(name, VIN, p_VIN)                                         \
		NETDEV_PARAMI(name, R,   p_R)                                           \
		NETDEV_PARAMI(name, N,   p_N)

#endif /* NLD_R2R_DAC_H_ */
