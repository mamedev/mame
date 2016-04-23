// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_opamps.h
 *
 */

//#pragma once

#ifndef NLD_OPAMPS_H_
#define NLD_OPAMPS_H_

#include "nl_base.h"
#include "nl_setup.h"
#include "nld_twoterm.h"
#include "nld_fourterm.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define OPAMP(_name, _model)                                                   \
		NET_REGISTER_DEV(OPAMP, _name)                                         \
		NETDEV_PARAMI(_name, MODEL, _model)

#define LM3900(_name)                                                          \
	SUBMODEL(opamp_lm3900, _name)

// ----------------------------------------------------------------------------------------
// Devices ...
// ----------------------------------------------------------------------------------------

NETLIST_EXTERNAL(opamp_lm3900)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_DEVICE_WITH_PARAMS(OPAMP,
	NETLIB_SUB(R) m_RP;
	NETLIB_SUB(C) m_CP;
	NETLIB_SUB(VCCS) m_G1;
	NETLIB_SUB(VCVS) m_EBUF;
	NETLIB_SUB(D) m_DP;
	NETLIB_SUB(D) m_DN;

	analog_input_t m_VCC;
	analog_input_t m_GND;

	param_model_t m_model;
	analog_output_t m_VH;
	analog_output_t m_VL;
	analog_output_t m_VREF;

	/* state */
	unsigned m_type;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_OPAMPS_H_ */
