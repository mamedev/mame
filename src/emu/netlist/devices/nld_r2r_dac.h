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

#include "../nl_base.h"
#include "../analog/nld_twoterm.h"

#define R2R_DAC(_name, _VIN, _R, _N)                                            \
		NET_REGISTER_DEV(R2R_DAC, _name)                                       \
		NETDEV_PARAMI(_name, VIN, _VIN)                                        \
		NETDEV_PARAMI(_name, R,   _R)                                          \
		NETDEV_PARAMI(_name, N,   _N)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_DEVICE_WITH_PARAMS_DERIVED(r2r_dac, twoterm,
	param_double_t m_VIN;
	param_double_t m_R;
	param_int_t m_num;
	param_int_t m_val;
);

NETLIB_NAMESPACE_DEVICES_END()


#endif /* NLD_R2R_DAC_H_ */
