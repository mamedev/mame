// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_R2R_DAC_H_
#define NLD_R2R_DAC_H_

#include "netlist/nl_setup.h"

#define R2R_DAC(name, p_VIN, p_R, p_N)                                          \
		NET_REGISTER_DEVEXT(R2R_DAC, name, p_VIN, p_R, p_N)

#endif /* NLD_R2R_DAC_H_ */
