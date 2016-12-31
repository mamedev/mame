// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_opamps.c
 *
 */

#include "nld_opamps.h"
#include "devices/net_lib.h"


namespace netlist
{
	namespace devices
	{
/*
 * Type = 0: Impedance changer
 *        1; Idealized opamp
 *        2; opamp with first pole
 *        3: opamp with first pole + output limit
 *        4: opamp with input stage, first pole + output limit
 *
 * Type 1 parameters:
 *     FPF = frequency of first pole in Hz (ony used for open-loop gain)
 *     UGF = unity gain frequency in Hz (only used for open-loop gain)
 *     RI = input resistance in Ohms
 *     RO = output resistance in Ohms
 *
 * Type 3 parameters:
 *     VLH = high supply rail minus high output swing in V
 *     VLL = low output swing minus low supply rail in V
 *     FPF = frequency of first pole in Hz
 *     UGF = unity gain frequency (transition frequency) in Hz
 *     SLEW = unity gain slew rate in V/s
 *     RI = input resistance in Ohms
 *     RO = output resistance in Ohms
 *     DAB = quiescent supply current in A
 */

/* .model abc OPAMP(VLH=2.0 VLL=0.2 FPF=5 UGF=10k SLEW=0.6u RI=1000k RO=50 DAB=0.002)
 *
 * Differential Amp Bias ~ op amp's total quiescent current.
 * */

NETLIB_UPDATE(OPAMP)
{
	const double cVt = 0.0258 * 1.0; // * m_n;
	const double cId = m_model.model_value("DAB"); // 3 mA
	const double cVd = cVt * std::log(cId / 1e-15 + 1.0);
	m_VH.push(m_VCC() - m_model.model_value("VLH") - cVd);
	m_VL.push(m_GND() + m_model.model_value("VLL") + cVd);
	m_VREF.push((m_VCC() + m_GND()) / 2.0);
}

NETLIB_RESET(OPAMP)
{
	m_G1.do_reset();
	m_G1.m_RI.setTo(m_model.model_value("RI"));

	if (m_type == 1)
	{
		double RO = m_model.model_value("RO");
		double G = m_model.model_value("UGF") / m_model.model_value("FPF") / RO;
		m_RP.set_R(RO);
		m_G1.m_G.setTo(G);
	}
	else if (m_type == 3)
	{
		m_EBUF->do_reset();
		m_DP->do_reset();
		m_DN->do_reset();
		m_CP->do_reset();
		m_RP.do_reset();

		m_EBUF->m_G.setTo(1.0);
		m_EBUF->m_RO.setTo(m_model.model_value("RO"));
		m_DP->m_model.setTo("D(IS=1e-15 N=1)");
		m_DN->m_model.setTo("D(IS=1e-15 N=1)");

		double CP = m_model.model_value("DAB") / m_model.model_value("SLEW");
		double RP = 0.5 / 3.1459 / CP / m_model.model_value("FPF");
		double G = m_model.model_value("UGF") / m_model.model_value("FPF") / RP;

		m_CP->m_C.setTo(CP);
		m_RP.set_R(RP);
		m_G1.m_G.setTo(G);

	}
}

/*
NETLIB_DEVICE_WITH_PARAMS(OPAMPx,
    NETLIB_NAME(R) m_RP;
    NETLIB_NAME(C) m_CP;
    NETLIB_NAME(VCCS) m_G1;
    NETLIB_NAME(VCVS) m_EBUF;

    param_model_t m_model;
    analog_input_t m_VH;
    analog_input_t m_VL;
    analog_input_t m_VREF;
);
*/

	} //namespace devices
} // namespace netlist
