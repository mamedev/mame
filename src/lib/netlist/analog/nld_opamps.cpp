// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_opamps.c
 *
 */

#include "nld_opamps.h"
#include "netlist/nl_base.h"
#include "netlist/nl_errstr.h"
#include "nlid_fourterm.h"
#include "nlid_twoterm.h"

#include <cmath>

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

	/*! Class representing the opamp model parameters.
	 *  The opamp model was designed based on designs from
	 *  http://www.ecircuitcenter.com/Circuits/opmodel1/opmodel1.htm.
	 *  Currently 2 different types are supported: Type 1 and Type 3. Type 1
	 *  is less complex and should run faster than Type 3.
	 *
	 *  This is an extension to the traditional SPICE approach which
	 *  assumes that you will be using an manufacturer model. These models may
	 *  have copyrights incompatible with the netlist license. Thus they may not
	 *  be suitable for certain implementations of netlist.
	 *
	 *  For the typical use cases in low frequency (< 100 KHz) applications at
	 *  which netlist is targeted, this model is certainly suitable. All parameters
	 *  can be determined from a typical opamp datasheet.
	 *
	 *   |Type|name  |parameter                                      |units|default| example|
	 *   |:--:|:-----|:----------------------------------------------|:----|------:|-------:|
	 *   |  3 |TYPE  |Model Type, 1 and 3 are supported              |     |       |        |
	 *   |1,3 |FPF   |frequency of first pole                        |Hz   |       |100     |
	 *   |  3 |SLEW  |unity gain slew rate                           |V/s  |       |       1|
	 *   |1,3 |RI    |input resistance                               |Ohm  |       |1M      |
	 *   |1,3 |RO    |output resistance                              |Ohm  |       |50      |
	 *   |1,3 |UGF   |unity gain frequency (transition frequency)    |Hz   |       |1000    |
	 *   |  3 |VLL   |low output swing minus low supply rail         |V    |       |1.5     |
	 *   |  3 |VLH   |high supply rail minus high output swing       |V    |       |1.5     |
	 *   |  3 |DAB   |Differential Amp Bias - total quiescent current|A    |       |0.001   |
	 */

	class opamp_model_t : public param_model_t
	{
	public:
		opamp_model_t(device_t &device, const pstring &name, const pstring &val)
		: param_model_t(device, name, val)
		, m_TYPE(*this, "TYPE")
		, m_FPF(*this, "FPF")
		, m_SLEW(*this, "SLEW")
		, m_RI(*this, "RI")
		, m_RO(*this, "RO")
		, m_UGF(*this, "UGF")
		, m_VLL(*this, "VLL")
		, m_VLH(*this, "VLH")
		, m_DAB(*this, "DAB")
		{}

		value_t m_TYPE;   //!< Model Type, 1 and 3 are supported
		value_t m_FPF;    //!< frequency of first pole
		value_t m_SLEW;   //!< unity gain slew rate
		value_t m_RI;     //!< input resistance
		value_t m_RO;     //!< output resistance
		value_t m_UGF;    //!< unity gain frequency (transition frequency)
		value_t m_VLL;    //!< low output swing minus low supply rail
		value_t m_VLH;    //!< high supply rail minus high output swing
		value_t m_DAB;    //!< Differential Amp Bias - total quiescent current
	};


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
			if (m_type < 1 || m_type > 3)
				log().fatal(MF_UNKNOWN_OPAMP_TYPE(m_type));

			if (m_type == 1)
			{
				register_subalias("PLUS", "G1.IP");
				register_subalias("MINUS", "G1.IN");
				register_subalias("OUT", "G1.OP");

				connect("G1.ON", "VREF");
				connect("RP1.2", "VREF");
				connect("RP1.1", "G1.OP");

			}
			if (m_type == 2 || m_type == 3)
			{
				create_and_register_subdevice("CP1", m_CP);
				create_and_register_subdevice("EBUF", m_EBUF);

				register_subalias("PLUS", "G1.IP");
				register_subalias("MINUS", "G1.IN");

				connect("G1.ON", "VREF");
				connect("RP1.2", "VREF");
				connect("CP1.2", "VREF");
				connect("EBUF.ON", "VREF");
				connect("EBUF.IN", "VREF");

				connect("RP1.1", "G1.OP");
				connect("CP1.1", "RP1.1");

				connect("EBUF.IP", "RP1.1");
			}
			if (m_type == 2)
			{
				register_subalias("OUT", "EBUF.OP");
			}
			if (m_type == 3)
			{

				create_and_register_subdevice("DN", m_DN, "D(IS=1e-15 N=1)");
				create_and_register_subdevice("DP", m_DP, "D(IS=1e-15 N=1)");

				connect("DP.K", "VH");
				connect("VL", "DN.A");
				connect("DP.A", "DN.K");
				connect("DN.K", "RP1.1");

				register_subalias("OUT", "EBUF.OP");
			}

		}

		NETLIB_UPDATEI();
		NETLIB_RESETI()
		{
		}
		NETLIB_UPDATE_PARAMI();

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

	NETLIB_UPDATE_PARAM(opamp)
	{
		m_G1.m_RI.setTo(m_model.m_RI);

		if (m_type == 1)
		{
			double RO = m_model.m_RO;
			double G = m_model.m_UGF / m_model.m_FPF / RO;
			m_RP.set_R(RO);
			m_G1.m_G.setTo(G);
		}
		if (m_type == 3 || m_type == 2)
		{
			double CP = m_model.m_DAB / m_model.m_SLEW;
			double RP = 0.5 / constants::pi() / CP / m_model.m_FPF;
			double G = m_model.m_UGF / m_model.m_FPF / RP;

			//printf("OPAMP %s: %g %g %g\n", name().c_str(), CP, RP, G);
			if (m_model.m_SLEW / (4.0 * constants::pi() * 0.0258) < m_model.m_UGF)
				log().warning("Opamp <{1}> parameters fail convergence criteria", this->name());

			m_CP->m_C.setTo(CP);
			m_RP.set_R(RP);
			m_G1.m_G.setTo(G);

		}
		if (m_type == 2)
		{
			m_EBUF->m_G.setTo(1.0);
			m_EBUF->m_RO.setTo(m_model.m_RO);
		}
		if (m_type == 3)
		{
			m_EBUF->m_G.setTo(1.0);
			m_EBUF->m_RO.setTo(m_model.m_RO);
		}
	}


	} //namespace analog

	namespace devices {
		NETLIB_DEVICE_IMPL_NS(analog, opamp, "OPAMP", "MODEL")
	} // namespace devices
} // namespace netlist
