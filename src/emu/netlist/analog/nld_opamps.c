// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_opamps.c
 *
 */

#include "nld_opamps.h"
#include "../devices/net_lib.h"

NETLIST_START(opamp_lm3900)

	/*
	 *  Fast norton opamp model without bandwidth
	 */

	/* Terminal definitions for calling netlists */

	ALIAS(PLUS, R1.1) // Positive input
	ALIAS(MINUS, R2.1) // Negative input
	ALIAS(OUT, G1.OP) // Opamp output ...
	ALIAS(VM, G1.ON)  // V- terminal
	ALIAS(VP, DUMMY.I)  // V+ terminal

	DUMMY_INPUT(DUMMY)

	/* The opamp model */

	RES(R1, 1)
	RES(R2, 1)
	NET_C(R1.1, G1.IP)
	NET_C(R2.1, G1.IN)
	NET_C(R1.2, R2.2, G1.ON)
	VCVS(G1)
	PARAM(G1.G, 10000000)
	//PARAM(G1.RI, 1)
	PARAM(G1.RO, RES_K(8))

NETLIST_END()

NETLIB_NAMESPACE_DEVICES_START()

/*
 * Type = 0: Impedance changer
 * 		  1; Ideal opamp
 * 		  2; opamp with first pole
 * 		  3: opamp with first pole + output limit
 * 		  4: opamp with input stage, first pole + output limit
 */

NETLIB_START(OPAMP)
{
	register_sub("RP1", m_RP);
	register_sub("CP1", m_CP);
	register_sub("G1", m_G1);
	register_sub("EBUF", m_EBUF);
	register_sub("DN", m_DN);
	register_sub("DP", m_DP);

	register_input("VCC", m_VCC);
	register_input("GND", m_GND);

	register_output("VL", m_VL);
	register_output("VH", m_VH);
	register_output("VREF", m_VREF);

	register_param("MODEL", m_model, "");

	m_type = m_model.model_value("TYPE");

	if (m_type == 3)
	{
		register_subalias("PLUS", "G1.IP");
		register_subalias("MINUS", "G1.IN");
		register_subalias("OUT", "EBUF.OP");

		connect_late("EBUF.ON", "VREF");

		connect_late("G1.ON", "VREF");
		connect_late("RP1.2", "VREF");
		connect_late("CP1.2", "VREF");
		connect_late("EBUF.IN", "VREF");

		connect_late("RP1.1", "G1.OP");
		connect_late("CP1.1", "RP1.1");

		connect_late("DP.K", "VH");
		connect_late("VL", "DN.A");
		connect_late("DP.A", "DN.K");
		connect_late("DN.K", "RP1.1");
		connect_late("EBUF.IP", "RP1.1");
	}
	else
		netlist().error("Unknown opamp type: %d", m_type);
}

/* .model abc OPAMP(VLH=2.0 VLL=0.2 FPF=5 UGF=10k SLEW=0.6u RI=1000k RO=50 DAB=0.002)
 *
 * Differential Amp Bias ~ op amp's total quiescent current.
 * */

NETLIB_UPDATE(OPAMP)
{
	const double cVt = 0.0258 * 1.0; // * m_n;
	const double cId = m_model.model_value("DAB"); // 3 mA
	const double cVd = cVt * nl_math::log(cId / 1e-15 + 1.0);
	m_VH.set_Q(INPANALOG(m_VCC) - m_model.model_value("VLH") - cVd);
	m_VL.set_Q(INPANALOG(m_GND) + m_model.model_value("VLL") + cVd);
	m_VREF.set_Q((INPANALOG(m_VCC) + INPANALOG(m_GND)) / 2.0);
}

NETLIB_RESET(OPAMP)
{
	m_EBUF.do_reset();
	m_G1.do_reset();
	m_DP.do_reset();
	m_DN.do_reset();
	m_CP.do_reset();
	m_RP.do_reset();

	m_EBUF.m_G.setTo(1.0);
	m_G1.m_RI.setTo(m_model.model_value("RI"));
	m_EBUF.m_RO.setTo(m_model.model_value("RO"));
	m_DP.m_model.setTo("D(IS=1e-15 N=1)");
	m_DN.m_model.setTo("D(IS=1e-15 N=1)");

	double CP = m_model.model_value("DAB") / m_model.model_value("SLEW");
	double RP = 0.5 / 3.1459 / CP / m_model.model_value("FPF");
	double G = m_model.model_value("UGF") / m_model.model_value("FPF") / RP;

	//printf("CP=%e RP=%f G=%f\n", CP, RP, G);
	m_CP.m_C.setTo(CP);
	m_RP.set_R(RP);
	m_G1.m_G.setTo(G);

}

NETLIB_UPDATE_PARAM(OPAMP)
{
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

NETLIB_NAMESPACE_DEVICES_END()

