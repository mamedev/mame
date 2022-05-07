// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "nl_base.h"
#include "nl_errstr.h"
#include "nlid_fourterm.h"
#include "nlid_twoterm.h"

//
// Set to 1 to model output impedance as a series resistor.
// The default is that the VCVS already has an internal impedance.
// This needs more investigation.
//
#define TEST_ALT_OUTPUT (0)

namespace netlist
{
	namespace analog
	{

	/// \brief Class representing the opamp model parameters.
	///
	///  The opamp model was designed based on designs from
	///  http://www.ecircuitcenter.com/Circuits/opmodel1/opmodel1.htm.
	///  Currently 2 different types are supported: Type 1 and Type 3. Type 1
	///  is less complex and should run faster than Type 3.
	///
	///  This is an extension to the traditional SPICE approach which
	///  assumes that you will be using an manufacturer model. These models may
	///  have copyrights incompatible with the netlist license. Thus they may not
	///  be suitable for certain implementations of netlist.
	///
	///  For the typical use cases in low frequency (< 100 KHz) applications at
	///  which netlist is targeted, this model is certainly suitable. All parameters
	///  can be determined from a typical opamp datasheet.
	///
	///   |Type|name  |parameter                                      |units|default| example|
	///   |:--:|:-----|:----------------------------------------------|:----|------:|-------:|
	///   |  3 |TYPE  |Model Type, 1 and 3 are supported              |     |       |        |
	///   |1,3 |FPF   |frequency of first pole                        |Hz   |       |100     |
	///   |  3 |SLEW  |unity gain slew rate                           |V/s  |       |       1|
	///   |1,3 |RI    |input resistance                               |Ohm  |       |1M      |
	///   |1,3 |RO    |output resistance                              |Ohm  |       |50      |
	///   |1,3 |UGF   |unity gain frequency (transition frequency)    |Hz   |       |1000    |
	///   |  3 |VLL   |low output swing minus low supply rail         |V    |       |1.5     |
	///   |  3 |VLH   |high supply rail minus high output swing       |V    |       |1.5     |
	///   |  3 |DAB   |Differential Amp Bias - total quiescent current|A    |       |0.001   |
	///
	///
	/// Type = 0: Impedance changer
	///        1; Idealized opamp
	///        2; opamp with first pole
	///        3: opamp with first pole + output limit
	///        4: opamp with input stage, first pole + output limit
	///
	/// Type 1 parameters:
	///     FPF = frequency of first pole in Hz (ony used for open-loop gain)
	///     UGF = unity gain frequency in Hz (only used for open-loop gain)
	///     RI = input resistance in Ohms
	///     RO = output resistance in Ohms
	///
	/// Type 3 parameters:
	///     VLH = high supply rail minus high output swing in V
	///     VLL = low output swing minus low supply rail in V
	///     FPF = frequency of first pole in Hz
	///     UGF = unity gain frequency (transition frequency) in Hz
	///     SLEW = unity gain slew rate in V/s
	///     RI = input resistance in Ohms
	///     RO = output resistance in Ohms
	///     DAB = Differential Amp Bias ~ op amp's total quiescent current.
	///
	/// .model abc OPAMP(VLH=2.0 VLL=0.2 FPF=5 UGF=10k SLEW=0.6u RI=1000k RO=50 DAB=0.002)
	///
	/// http://www.ecircuitcenter.com/Circuits/opmodel1/opmodel1.htm
	///
	///
	class opamp_model_t
	{
	public:
		opamp_model_t(param_model_t &model)
		: m_TYPE(model, "TYPE")
		, m_FPF(model, "FPF")
		, m_SLEW(model, "SLEW")
		, m_RI(model, "RI")
		, m_RO(model, "RO")
		, m_UGF(model, "UGF")
		, m_VLL(model, "VLL")
		, m_VLH(model, "VLH")
		, m_DAB(model, "DAB")
		{}

		param_model_t::value_t m_TYPE;   //!< Model Type, 1 and 3 are supported
		param_model_t::value_t m_FPF;    //!< frequency of first pole
		param_model_t::value_t m_SLEW;   //!< unity gain slew rate
		param_model_t::value_t m_RI;     //!< input resistance
		param_model_t::value_t m_RO;     //!< output resistance
		param_model_t::value_t m_UGF;    //!< unity gain frequency (transition frequency)
		param_model_t::value_t m_VLL;    //!< low output swing minus low supply rail
		param_model_t::value_t m_VLH;    //!< high supply rail minus high output swing
		param_model_t::value_t m_DAB;    //!< Differential Amp Bias - total quiescent current
	};


	NETLIB_BASE_OBJECT(opamp)
	{
		NETLIB_CONSTRUCTOR(opamp)
		, m_RP(*this, "RP1")
		, m_G1(*this, "G1")
		, m_VCC(*this, "VCC", NETLIB_DELEGATE(supply))
		, m_GND(*this, "GND", NETLIB_DELEGATE(supply))
		, m_model(*this, "MODEL", "LM324")
		, m_modacc(m_model)
		, m_VH(*this, "VH")
		, m_VL(*this, "VL")
		, m_VREF(*this, "VREF")
		{
			m_type = plib::narrow_cast<int>(m_modacc.m_TYPE);
			if (m_type < 1 || m_type > 3)
			{
				log().fatal(MF_OPAMP_UNKNOWN_TYPE(m_type));
				throw nl_exception(MF_OPAMP_UNKNOWN_TYPE(m_type));
			}

			if (m_type == 1)
			{
				register_sub_alias("PLUS", "G1.IP");
				register_sub_alias("MINUS", "G1.IN");
				register_sub_alias("OUT", "G1.OP");

				connect("G1.ON", "VREF");
				connect("RP1.2", "VREF");
				connect("RP1.1", "G1.OP");

			}
			if (m_type == 2 || m_type == 3)
			{
				create_and_register_sub_device(*this, "CP1", m_CP);
				create_and_register_sub_device(*this, "EBUF", m_EBUF);
#if TEST_ALT_OUTPUT
				create_and_register_sub_device("RO", m_RO);
#endif
				register_sub_alias("PLUS", "G1.IP");
				register_sub_alias("MINUS", "G1.IN");

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
#if TEST_ALT_OUTPUT
				connect("EBUF.OP", "RO.1");
				register_sub_alias("OUT", "RO.2");
#else
				register_sub_alias("OUT", "EBUF.OP");
#endif
			}
			if (m_type == 3)
			{

				create_and_register_sub_device(*this, "DN", m_DN, "D(IS=1e-15 N=1)");
				create_and_register_sub_device(*this, "DP", m_DP, "D(IS=1e-15 N=1)");

				connect("DP.K", "VH");
				connect("VL", "DN.A");
				connect("DP.A", "DN.K");
				connect("DN.K", "RP1.1");
#if TEST_ALT_OUTPUT
				connect("EBUF.OP", "RO.1");
				register_sub_alias("OUT", "RO.2");
#else
				register_sub_alias("OUT", "EBUF.OP");
#endif
			}

		}

		NETLIB_HANDLERI(supply)
		{
			const nl_fptype cVt = nlconst::np_VT(nlconst::one()); // * m_n;
			const nl_fptype cId = m_modacc.m_DAB; // 3 mA
			const nl_fptype cVd = cVt * plib::log(cId / nlconst::np_Is() + nlconst::one());

			m_VH.push(m_VCC() - m_modacc.m_VLH - cVd);
			m_VL.push(m_GND() + m_modacc.m_VLL + cVd);
			m_VREF.push((m_VCC() + m_GND()) / nlconst::two());
		}

		NETLIB_RESETI()
		{
		}

		NETLIB_UPDATE_PARAMI();

	private:

		analog::NETLIB_SUB(R_base) m_RP;
		analog::NETLIB_SUB(VCCS) m_G1;
		NETLIB_SUB_UPTR(analog, C) m_CP;
#if TEST_ALT_OUTPUT
		NETLIB_SUB_UPTR(analog, R_base) m_RO;
#endif
		NETLIB_SUB_UPTR(analog, VCVS) m_EBUF;
		NETLIB_SUB_UPTR(analog, D) m_DP;
		NETLIB_SUB_UPTR(analog, D) m_DN;

		analog_input_t m_VCC;
		analog_input_t m_GND;

		param_model_t m_model;
		opamp_model_t m_modacc;
		analog_output_t m_VH;
		analog_output_t m_VL;
		analog_output_t m_VREF;

		// state
		int m_type;
	};

	NETLIB_UPDATE_PARAM(opamp)
	{
		m_G1.m_RI.set(m_modacc.m_RI);

		if (m_type == 1)
		{
			nl_fptype RO = m_modacc.m_RO;
			nl_fptype G = m_modacc.m_UGF / m_modacc.m_FPF / RO;
			m_RP.set_R(RO);
			m_G1.m_G.set(G);
		}
		if (m_type == 3 || m_type == 2)
		{
			nl_fptype CP = m_modacc.m_DAB / m_modacc.m_SLEW;
			nl_fptype RP = nlconst::half() / nlconst::pi() / CP / m_modacc.m_FPF;
			nl_fptype G = m_modacc.m_UGF / m_modacc.m_FPF / RP;

			//printf("OPAMP %s: %g %g %g\n", name().c_str(), CP, RP, G);
			if (m_modacc.m_SLEW / (nlconst::four() * nlconst::pi() * nlconst::np_VT()) < m_modacc.m_UGF)
				log().warning(MW_OPAMP_FAIL_CONVERGENCE(this->name()));

			m_CP->set_cap_embedded(CP);
			m_RP.set_R(RP);
			m_G1.m_G.set(G);

		}
		if (m_type == 2)
		{
			m_EBUF->m_G.set(nlconst::one());
#if TEST_ALT_OUTPUT
			m_EBUF->m_RO.set(0.001);
			m_RO->set_R(m_modacc.m_RO);
#else
			m_EBUF->m_RO.set(m_modacc.m_RO);
#endif
		}
		if (m_type == 3)
		{
			m_EBUF->m_G.set(nlconst::one());
#if TEST_ALT_OUTPUT
			m_EBUF->m_RO.set(0.001);
			m_RO->set_R(m_modacc.m_RO);
#else
			m_EBUF->m_RO.set(m_modacc.m_RO);
#endif
		}
	}


	} //namespace analog

	namespace devices {
		NETLIB_DEVICE_IMPL_NS(analog, opamp, "OPAMP", "MODEL")
	} // namespace devices
} // namespace netlist
