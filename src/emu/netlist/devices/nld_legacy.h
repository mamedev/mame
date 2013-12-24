// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_legacy.h
 *
 * All of the devices below needs to disappear at some time .....
 *
 *
 */

#pragma once

#ifndef NLD_LEGACY_H_
#define NLD_LEGACY_H_

#include "../nl_base.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define NETDEV_RSFF(_name, _S, _R)                                                  \
		NET_REGISTER_DEV(nicRSFF, _name)                                            \
		NET_CONNECT(_name, S, _S)                                                   \
		NET_CONNECT(_name, R, _R)


#define NE555N_MSTABLE(_name, _TRIG, _CV)                                           \
		NET_REGISTER_DEV(nicNE555N_MSTABLE, _name)                                  \
		NET_CONNECT(_name, TRIG, _TRIG)                                             \
		NET_CONNECT(_name, CV, _CV)

#define NETDEV_MIXER3(_name, _I1, _I2, _I3)                                         \
		NET_REGISTER_DEV(nicMixer8, _name)                                          \
		NET_CONNECT(_name, I1, _I1)                                                 \
		NET_CONNECT(_name, I2, _I2)                                                 \
		NET_CONNECT(_name, I3, _I3)

#define NETDEV_SWITCH2(_name, _i1, _i2)                                             \
		NET_REGISTER_DEV(nicMultiSwitch, _name)                                     \
		NET_CONNECT(_name, i1, _i1)                                                 \
		NET_CONNECT(_name, i2, _i2)

// ----------------------------------------------------------------------------------------
// Devices ...
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE(nicRSFF,
	netlist_ttl_input_t m_S;
	netlist_ttl_input_t m_R;

	netlist_ttl_output_t m_Q;
	netlist_ttl_output_t m_QQ;
);

NETLIB_DEVICE_WITH_PARAMS(nicMixer8,
	netlist_analog_input_t m_I[8];

	netlist_analog_output_t m_Q;
	netlist_analog_output_t m_low;

	netlist_param_double_t m_R[8];

	double m_w[8];
);

NETLIB_DEVICE_WITH_PARAMS(nicNE555N_MSTABLE,

	//ATTR_HOT void timer_cb(INT32 timer_id);

	netlist_analog_input_t m_trigger;
	netlist_analog_input_t m_CV;
	netlist_analog_input_t m_THRESHOLD; /* internal */

	bool m_last;

	netlist_analog_output_t m_Q;
	netlist_analog_output_t m_THRESHOLD_OUT; /* internal */

	//netlist_base_timer_t *m_timer;
	netlist_param_double_t m_R;
	netlist_param_double_t m_C;
	netlist_param_double_t m_VS;
	netlist_param_double_t m_VL;

	double nicNE555N_cv();
	double nicNE555N_clamp(const double v, const double a, const double b);

);

NETLIB_DEVICE_WITH_PARAMS(nicMultiSwitch,
	netlist_analog_input_t m_I[8];

	netlist_analog_output_t m_Q;
	netlist_analog_output_t m_low;

	netlist_param_int_t m_POS;

	int m_position;
);




#endif /* NLD_LEGACY_H_ */
