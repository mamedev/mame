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

#define OPAMP(name, model)                                                     \
		NET_REGISTER_DEV(OPAMP, name)                                          \
		NETDEV_PARAMI(name, MODEL, model)

// ----------------------------------------------------------------------------------------
// Devices ...
// ----------------------------------------------------------------------------------------

namespace netlist
{
	namespace devices
	{
NETLIB_OBJECT(OPAMP)
{
	NETLIB_CONSTRUCTOR(OPAMP)
	, m_RP(*this, "RP1")
	, m_G1(*this, "G1")
	, m_VCC(*this, "VCC")
	, m_GND(*this, "GND")
	, m_model(*this, "MODEL", "LM324")
	, m_VH(*this, "VH")
	, m_VL(*this, "VL")
	, m_VREF(*this, "VREF")
	{
		m_type = static_cast<int>(m_model.model_value("TYPE"));

		if (m_type == 1)
		{
			register_subalias("PLUS", "G1.IP");
			register_subalias("MINUS", "G1.IN");
			register_subalias("OUT", "G1.OP");

			connect("G1.ON", "VREF");
			connect("RP1.2", "VREF");
			connect("RP1.1", "G1.OP");

		}
		else if (m_type == 3)
		{
			register_sub("CP1", m_CP);
			register_sub("EBUF", m_EBUF);
			register_sub("DN", m_DN);
			register_sub("DP", m_DP);

			m_DP->m_model.setTo("D(IS=1e-15 N=1)");
			m_DN->m_model.setTo("D(IS=1e-15 N=1)");

			register_subalias("PLUS", "G1.IP");
			register_subalias("MINUS", "G1.IN");
			register_subalias("OUT", "EBUF.OP");

			connect("EBUF.ON", "VREF");

			connect("G1.ON", "VREF");
			connect("RP1.2", "VREF");
			connect("CP1.2", "VREF");
			connect("EBUF.IN", "VREF");

			connect("RP1.1", "G1.OP");
			connect("CP1.1", "RP1.1");

			connect("DP.K", "VH");
			connect("VL", "DN.A");
			connect("DP.A", "DN.K");
			connect("DN.K", "RP1.1");
			connect("EBUF.IP", "RP1.1");
		}
		else
			netlist().log().fatal("Unknown opamp type: {1}", m_type);

	}

	NETLIB_UPDATEI();
	NETLIB_RESETI();
	NETLIB_UPDATE_PARAMI()
	{
	}

private:

	NETLIB_SUB(R_base) m_RP;
	NETLIB_SUB(VCCS) m_G1;
	NETLIB_SUBXX(C) m_CP;
	NETLIB_SUBXX(VCVS) m_EBUF;
	NETLIB_SUBXX(D) m_DP;
	NETLIB_SUBXX(D) m_DN;

	analog_input_t m_VCC;
	analog_input_t m_GND;

	param_model_t m_model;
	analog_output_t m_VH;
	analog_output_t m_VL;
	analog_output_t m_VREF;

	/* state */
	int m_type;
};

	} //namespace devices
} // namespace netlist

#endif /* NLD_OPAMPS_H_ */
