// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_opamps.c
 *
 */

#include "devices/net_lib.h"

#include "nld_opamps.h"


namespace netlist
{
	namespace analog
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
	 *     DAB = Differential Amp Bias ~ op amp's total quiescent current.
	 *
	 * .model abc OPAMP(VLH=2.0 VLL=0.2 FPF=5 UGF=10k SLEW=0.6u RI=1000k RO=50 DAB=0.002)
	 *
	 * http://www.ecircuitcenter.com/Circuits/opmodel1/opmodel1.htm
	 *
	 * */


	NETLIB_OBJECT(opamp)
	{
		NETLIB_CONSTRUCTOR(opamp)
		, m_RP(*this, "RP1")
		, m_G1(*this, "G1")
		, m_VCC(*this, "VCC")
		, m_GND(*this, "GND")
		, m_model(*this, "MODEL", "LM324")
		, m_VH(*this, "VH")
		, m_VL(*this, "VL")
		, m_VREF(*this, "VREF")
		{
			m_type = static_cast<int>(m_model.m_TYPE);

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
				register_sub("DN", m_DN, "D(IS=1e-15 N=1)");
				register_sub("DP", m_DP, "D(IS=1e-15 N=1)");

				//m_DP->m_model.setTo("D(IS=1e-15 N=1)");
				//m_DN->m_model.setTo("D(IS=1e-15 N=1)");

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

		analog::NETLIB_SUB(R_base) m_RP;
		analog::NETLIB_SUB(VCCS) m_G1;
		NETLIB_SUBXX(analog, C) m_CP;
		NETLIB_SUBXX(analog, VCVS) m_EBUF;
		NETLIB_SUBXX(analog, D) m_DP;
		NETLIB_SUBXX(analog, D) m_DN;

		analog_input_t m_VCC;
		analog_input_t m_GND;

		opamp_model_t m_model;
		analog_output_t m_VH;
		analog_output_t m_VL;
		analog_output_t m_VREF;

		/* state */
		int m_type;
	};

	NETLIB_UPDATE(opamp)
	{
		const double cVt = 0.0258 * 1.0; // * m_n;
		const double cId = m_model.m_DAB; // 3 mA
		const double cVd = cVt * std::log(cId / 1e-15 + 1.0);
		m_VH.push(m_VCC() - m_model.m_VLH - cVd);
		m_VL.push(m_GND() + m_model.m_VLL + cVd);
		m_VREF.push((m_VCC() + m_GND()) / 2.0);
	}

	NETLIB_RESET(opamp)
	{
		m_G1.do_reset();
		m_G1.m_RI.setTo(m_model.m_RI);

		if (m_type == 1)
		{
			double RO = m_model.m_RO;
			double G = m_model.m_UGF / m_model.m_FPF / RO;
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
			m_EBUF->m_RO.setTo(m_model.m_RO);

			double CP = m_model.m_DAB / m_model.m_SLEW;
			double RP = 0.5 / 3.1459 / CP / m_model.m_FPF;
			double G = m_model.m_UGF / m_model.m_FPF / RP;

			//printf("Min Freq %s: %f\n", name().c_str(), 1.0 / (CP*RP / (G*RP)));

			m_CP->m_C.setTo(CP);
			m_RP.set_R(RP);
			m_G1.m_G.setTo(G);

		}
	}


	} //namespace analog

	namespace devices {
		NETLIB_DEVICE_IMPL_NS(analog, opamp)
	}
} // namespace netlist
