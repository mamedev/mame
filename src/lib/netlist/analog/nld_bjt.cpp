// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "solver/nld_solver.h"
#include "nl_base.h"
#include "nlid_twoterm.h"

// Names
// spell-checker: words Ebers, Moll

// FIXME: Remove QBJT_switch - no more use

namespace netlist
{
namespace analog
{
	class diode
	{
	public:
		diode()
		: m_Is(nlconst::np_Is())
		, m_VT(nlconst::np_VT())
		, m_VT_inv(plib::reciprocal(m_VT))
		{}

		diode(nl_fptype Is, nl_fptype n)
		: m_Is(Is)
		, m_VT(nlconst::np_VT(n))
		, m_VT_inv(plib::reciprocal(m_VT))
		{
		}
		void set(nl_fptype Is, nl_fptype n) noexcept
		{
			m_Is = Is;
			m_VT = nlconst::np_VT(n);
			m_VT_inv = plib::reciprocal(m_VT);
		}
		nl_fptype I(nl_fptype V) const noexcept { return m_Is * plib::exp(V * m_VT_inv) - m_Is; }
		nl_fptype g(nl_fptype V) const noexcept { return m_Is * m_VT_inv * plib::exp(V * m_VT_inv); }
		nl_fptype V(nl_fptype I) const noexcept { return plib::log1p(I / m_Is) * m_VT; } // log1p(x)=log(1.0 + x)
		nl_fptype gI(nl_fptype I) const noexcept { return m_VT_inv * (I + m_Is); }

	private:
		nl_fptype m_Is;
		nl_fptype m_VT;
		nl_fptype m_VT_inv;
	};

	// -----------------------------------------------------------------------------
	// nld_Q - Base classes
	// -----------------------------------------------------------------------------

	enum class bjt_type {
		BJT_NPN,
		BJT_PNP
	};

	/// \brief Class representing the bjt model parameters.
	///
	///  This is the model representation of the bjt model. Typically, SPICE uses
	///  the following parameters. A "Y" in the first column indicates that the
	///  parameter is actually used in netlist.
	///
	/// | NL? | name | parameter                                                             | units |  default |         example | area |
	/// |:---:|------|-----------------------------------------------------------------------|-------|---------:|----------------:|:----:|
	/// |  Y  | IS   | transport saturation current                                          | A     |   1E-016 |          1E-015 |   *  |
	/// |  Y  | BF   | ideal maximum forward beta                                            | -     |      100 |             100 |      |
	/// |  Y  | NF   | forward current emission coefficient                                  | -     |        1 |               1 |      |
	/// |     | VAF  | forward Early voltage                                                 | V     | infinite |             200 |      |
	/// |     | IKF  | corner for forward beta high current roll-off                         | A     | infinite |            0.01 |   *  |
	/// |     | ISE  | B-E leakage saturation current                                        | A     |        0 | 0.0000000000001 |   *  |
	/// |     | NE   | B-E leakage emission coefficient                                      | -     |      1.5 |               2 |      |
	/// |  Y  | BR   | ideal maximum reverse beta                                            | -     |        1 |             0.1 |      |
	/// |  Y  | NR   | reverse current emission coefficient                                  | -     |        1 |               1 |      |
	/// |     | VAR  | reverse Early voltage                                                 | V     | infinite |             200 |      |
	/// |     | IKR  | corner for reverse beta high current roll-off                         | A     | infinite |            0.01 |   *  |
	/// |     | ISC  | leakage saturation current                                            | A     |        0 |               8 |      |
	/// |     | NC   | leakage emission coefficient                                          | -     |        2 |             1.5 |      |
	/// |     | RB   | zero bias base resistance                                             |       |        0 |             100 |   *  |
	/// |     | IRB  | current where base resistance falls halfway to its min value          | A     | infinite |             0.1 |   *  |
	/// |     | RBM  | minimum base resistance at high currents                              |       |       RB |              10 |   *  |
	/// |     | RE   | emitter resistance                                                    |       |        0 |               1 |   *  |
	/// |     | RC   | collector resistance                                                  |       |        0 |              10 |   *  |
	/// |  Y  | CJE  | B-E zero-bias depletion capacitance                                   | F     |        0 |             2pF |   *  |
	/// |     | VJE  | B-E built-in potential                                                | V     |     0.75 |             0.6 |      |
	/// |     | MJE  | B-E junction exponential factor                                       | -     |     0.33 |            0.33 |      |
	/// |     | TF   | ideal forward transit time                                            | sec   |        0 |           0.1ns |      |
	/// |     | XTF  | coefficient for bias dependence of TF                                 | -     |        0 |                 |      |
	/// |     | VTF  | voltage describing VBC  dependence of TF                              | V     | infinite |                 |      |
	/// |     | ITF  | high-current parameter  for effect on TF                              | A     |        0 |                 |   *  |
	/// |     | PTF  | excess phase at freq=1.0/(TF*2PI) Hz                                  | deg   |        0 |                 |      |
	/// |  Y  | CJC  | B-C zero-bias depletion capacitance                                   | F     |        0 |             2pF |   *  |
	/// |     | VJC  | B-C built-in potential                                                | V     |     0.75 |             0.5 |      |
	/// |     | MJC  | B-C junction exponential factor                                       | -     |     0.33 |             0.5 |      |
	/// |     | XCJC | fraction of B-C depletion capacitance connected to internal base node | -     |        1 |                 |      |
	/// |     | TR   | ideal reverse transit time                                            | sec   |        0 |            10ns |      |
	/// |     | CJS  | zero-bias collector-substrate capacitance                             | F     |        0 |             2pF |   *  |
	/// |     | VJS  | substrate junction built-in potential                                 | V     |     0.75 |                 |      |
	/// |     | MJS  | substrate junction exponential factor                                 | -     |        0 |             0.5 |      |
	/// |     | XTB  | forward and reverse beta temperature exponent                         | -     |        0 |                 |      |
	/// |     | EG   | energy gap for temperature effect on IS                               | eV    |     1.11 |                 |      |
	/// |     | XTI  | temperature exponent for effect on IS                                 | -     |        3 |                 |      |
	/// |     | KF   | flicker-noise coefficient                                             | -     |        0 |                 |      |
	/// |     | AF   | flicker-noise exponent                                                | -     |        1 |                 |      |
	/// |     | FC   | coefficient for forward-bias depletion capacitance formula            | -     |      0.5 |                 |      |
	/// |     | TNOM | Parameter measurement temperature                                     | C     |       27 |              50 |      |
	///
	class bjt_model_t
	{
	public:
		bjt_model_t(param_model_t &model)
		: m_type((model.type() == "NPN") ? bjt_type::BJT_NPN : bjt_type::BJT_PNP)
		, m_IS (model, "IS")
		, m_BF (model, "BF")
		, m_NF (model, "NF")
		, m_BR (model, "BR")
		, m_NR (model, "NR")
		, m_CJE(model, "CJE")
		, m_CJC(model, "CJC")
		{}

		bjt_type m_type;
		param_model_t::value_t m_IS;  //!< transport saturation current
		param_model_t::value_t m_BF;  //!< ideal maximum forward beta
		param_model_t::value_t m_NF;  //!< forward current emission coefficient
		param_model_t::value_t m_BR;  //!< ideal maximum reverse beta
		param_model_t::value_t m_NR;  //!< reverse current emission coefficient
		param_model_t::value_t m_CJE; //!< B-E zero-bias depletion capacitance
		param_model_t::value_t m_CJC; //!< B-C zero-bias depletion capacitance

	};

	// Have a common start for transistors

	NETLIB_BASE_OBJECT(QBJT)
	{
	public:

		NETLIB_CONSTRUCTOR_EX(QBJT, const pstring &model = "NPN")
		, m_model(*this, "MODEL", model)
		, m_qtype(bjt_type::BJT_NPN)
		{
		}

		NETLIB_IS_DYNAMIC(true)

		//NETLIB_RESETI();

		bjt_type qtype() const noexcept { return m_qtype; }
		bool is_qtype(bjt_type atype) const noexcept { return m_qtype == atype; }
		void set_qtype(bjt_type atype) noexcept { m_qtype = atype; }
	protected:

		param_model_t m_model;
	private:
		bjt_type m_qtype;
	};

	// -----------------------------------------------------------------------------
	// nld_QBJT_switch
	// -----------------------------------------------------------------------------


	//
	//         + -              C
	//   B ----VVV----+         |
	//                |         |
	//                Rb        Rc
	//                Rb        Rc
	//                Rb        Rc
	//                |         |
	//                +----+----+
	//                     |
	//                     E
	//

	NETLIB_OBJECT_DERIVED(QBJT_switch, QBJT)
	{
		NETLIB_CONSTRUCTOR(QBJT_switch)
		, m_bjt_model(m_model)
		, m_RB(*this, "m_RB", NETLIB_DELEGATE(terminal_handler))
		, m_RC(*this, "m_RC", NETLIB_DELEGATE(terminal_handler))
		, m_BC(*this, "m_BC", NETLIB_DELEGATE(terminal_handler))
		, m_gB(nlconst::cgmin())
		, m_gC(nlconst::cgmin())
		, m_V(nlconst::zero())
		, m_state_on(*this, "m_state_on", 0U)
		{
			register_sub_alias("B", m_RB.P());
			register_sub_alias("E", m_RB.N());
			register_sub_alias("C", m_RC.P());

			connect(m_RB.N(), m_RC.N());
			connect(m_RB.P(), m_BC.P());
			connect(m_RC.P(), m_BC.N());
		}

		NETLIB_RESETI();
		NETLIB_HANDLERI(terminal_handler)
		{
			auto *solver(m_RB.solver());
			if (solver != nullptr)
				solver->solve_now();
			else
				m_RC.solver()->solve_now();
		}

		NETLIB_UPDATE_PARAMI();
		NETLIB_UPDATE_TERMINALSI();

	private:
		bjt_model_t m_bjt_model;
		nld_two_terminal m_RB;
		nld_two_terminal m_RC;
		nld_two_terminal m_BC;

		nl_fptype m_gB; // base conductance / switch on
		nl_fptype m_gC; // collector conductance / switch on
		nl_fptype m_V; // internal voltage source
		state_var<unsigned> m_state_on;

	};

	// -----------------------------------------------------------------------------
	// nld_QBJT_EB
	// -----------------------------------------------------------------------------


	NETLIB_OBJECT_DERIVED(QBJT_EB, QBJT)
	{
	public:
		NETLIB_CONSTRUCTOR(QBJT_EB)
		, m_bjt_model(m_model)
		, m_gD_BC(*this, "m_D_BC")
		, m_gD_BE(*this, "m_D_BE")
		, m_D_CB(*this, "m_D_CB", NETLIB_DELEGATE(terminal_handler))
		, m_D_EB(*this, "m_D_EB", NETLIB_DELEGATE(terminal_handler))
		, m_D_EC(*this, "m_D_EC", NETLIB_DELEGATE(terminal_handler))
		, m_alpha_f(0)
		, m_alpha_r(0)
		{
			register_sub_alias("E", m_D_EB.P());   // Cathode
			register_sub_alias("B", m_D_EB.N());   // Anode

			register_sub_alias("C", m_D_CB.P());   // Cathode

			connect(m_D_EB.P(), m_D_EC.P());
			connect(m_D_EB.N(), m_D_CB.N());
			connect(m_D_CB.P(), m_D_EC.N());

			if (m_bjt_model.m_CJE > nlconst::zero())
			{
				create_and_register_sub_device(*this, "m_CJE", m_CJE);
				connect("B", "m_CJE.1");
				connect("E", "m_CJE.2");
			}
			if (m_bjt_model.m_CJC > nlconst::zero())
			{
				create_and_register_sub_device(*this, "m_CJC", m_CJC);
				connect("B", "m_CJC.1");
				connect("C", "m_CJC.2");
			}

		}

	protected:

		NETLIB_RESETI();
		NETLIB_HANDLERI(terminal_handler)
		{
			auto *solver(m_D_EB.solver());
			if (solver != nullptr)
				solver->solve_now();
			else
				m_D_CB.solver()->solve_now();
		}

		NETLIB_UPDATE_PARAMI();
		NETLIB_UPDATE_TERMINALSI();

	private:
		bjt_model_t m_bjt_model;
		generic_diode<diode_e::BIPOLAR> m_gD_BC;
		generic_diode<diode_e::BIPOLAR> m_gD_BE;

		nld_two_terminal m_D_CB;  // gcc, gce - gcc, gec - gcc, gcc - gce | Ic
		nld_two_terminal m_D_EB;  // gee, gec - gee, gce - gee, gee - gec | Ie
		nld_two_terminal m_D_EC;  // 0, -gec, -gcc, 0 | 0

		nl_fptype m_alpha_f;
		nl_fptype m_alpha_r;

		NETLIB_SUB_UPTR(analog, C) m_CJE;
		NETLIB_SUB_UPTR(analog, C) m_CJC;
	};


	// ----------------------------------------------------------------------------------------
	// nld_QBJT_switch
	// ----------------------------------------------------------------------------------------


	NETLIB_RESET(QBJT_switch)
	{
		if (m_RB.solver() == nullptr && m_RC.solver() == nullptr)
			throw nl_exception(MF_DEVICE_FRY_1(this->name()));
		NETLIB_NAME(QBJT)::reset();
		const auto zero(nlconst::zero());

		m_state_on = 0;

		m_RB.set_G_V_I(exec().gmin(), zero, zero);
		m_RC.set_G_V_I(exec().gmin(), zero, zero);

		m_BC.set_G_V_I(exec().gmin() / nlconst::magic(10.0), zero, zero);

	}

	NETLIB_UPDATE_PARAM(QBJT_switch)
	{
		nl_fptype IS = m_bjt_model.m_IS;
		nl_fptype BF = m_bjt_model.m_BF;
		nl_fptype NF = m_bjt_model.m_NF;
		//nl_fptype VJE = m_bjt_model.dValue("VJE", 0.75);

		// FIXME: check for PNP as well and bail out
		set_qtype(m_bjt_model.m_type);

		nl_fptype alpha = BF / (nlconst::one() + BF);

		diode d(IS, NF);

		// Assume 5mA Collector current for switch operation

		const auto cc(nlconst::magic(0.005));
		m_V = d.V(cc / alpha);

		// Base current is 0.005 / beta
		// as a rough estimate, we just scale the conductance down

		m_gB = plib::reciprocal((m_V/(cc / BF)));

		//m_gB = d.gI(0.005 / alpha);

		if (m_gB < exec().gmin())
			m_gB = exec().gmin();
		m_gC =  d.gI(cc); // very rough estimate
	}

	NETLIB_UPDATE_TERMINALS(QBJT_switch)
	{
		const nl_fptype m = (is_qtype( bjt_type::BJT_NPN) ? 1 : -1);

		const unsigned new_state = (m_RB.deltaV() * m > m_V ) ? 1 : 0;
		if (m_state_on ^ new_state)
		{
			const auto zero(nlconst::zero());
			const nl_fptype gb = new_state ? m_gB : exec().gmin();
			const nl_fptype gc = new_state ? m_gC : exec().gmin();
			const nl_fptype v  = new_state ? m_V * m : zero;

			m_RB.set_G_V_I(gb,   v,   zero);
			m_RC.set_G_V_I(gc, zero, zero);
			m_state_on = new_state;
		}
	}


	// ----------------------------------------------------------------------------------------
	// nld_Q - Ebers Moll
	// ----------------------------------------------------------------------------------------

	NETLIB_RESET(QBJT_EB)
	{
		if (m_D_EB.solver() == nullptr && m_D_CB.solver() == nullptr)
			throw nl_exception(MF_DEVICE_FRY_1(this->name()));
		NETLIB_NAME(QBJT)::reset();
		if (m_CJE)
		{
			m_CJE->reset();
			m_CJE->set_cap_embedded(m_bjt_model.m_CJE);
		}
		if (m_CJC)
		{
			m_CJC->reset();
			m_CJC->set_cap_embedded(m_bjt_model.m_CJC);
		}
	}

	NETLIB_UPDATE_TERMINALS(QBJT_EB)
	{
		const nl_fptype polarity(qtype() == bjt_type::BJT_NPN ? nlconst::one() : -nlconst::one());

		m_gD_BE.update_diode(-m_D_EB.deltaV() * polarity);
		m_gD_BC.update_diode(-m_D_CB.deltaV() * polarity);

		const nl_fptype gee = m_gD_BE.G();
		const nl_fptype gcc = m_gD_BC.G();
		const nl_fptype gec =  m_alpha_r * gcc;
		const nl_fptype gce =  m_alpha_f * gee;
		const nl_fptype sIe = -m_gD_BE.I() + m_alpha_r * m_gD_BC.I();
		const nl_fptype sIc = m_alpha_f * m_gD_BE.I() - m_gD_BC.I();
		const nl_fptype Ie = (sIe + gee * m_gD_BE.Vd() - gec * m_gD_BC.Vd()) * polarity;
		const nl_fptype Ic = (sIc - gce * m_gD_BE.Vd() + gcc * m_gD_BC.Vd()) * polarity;

		// "Circuit Design", page 174

		m_D_EB.set_mat(      gee, gec - gee,  -Ie,
					   gce - gee, gee - gec,   Ie);
		m_D_CB.set_mat(      gcc, gce - gcc,  -Ic,
					   gec - gcc, gcc - gce,   Ic);
		m_D_EC.set_mat(        0,      -gec,    0,
							-gce,         0,    0);
	}

	NETLIB_UPDATE_PARAM(QBJT_EB)
	{
		nl_fptype IS = m_bjt_model.m_IS;
		nl_fptype BF = m_bjt_model.m_BF;
		nl_fptype NF = m_bjt_model.m_NF;
		nl_fptype BR = m_bjt_model.m_BR;
		nl_fptype NR = m_bjt_model.m_NR;
		//nl_fptype VJE = m_m_bjt_model.dValue("VJE", 0.75);

		// FIXME: check for PNP as well and bail out
		set_qtype(m_bjt_model.m_type);

		m_alpha_f = BF / (nlconst::one() + BF);
		m_alpha_r = BR / (nlconst::one() + BR);

		m_gD_BE.set_param(IS / m_alpha_f, NF, exec().gmin(), nlconst::T0());
		m_gD_BC.set_param(IS / m_alpha_r, NR, exec().gmin(), nlconst::T0());
	}

} // namespace analog

namespace devices {
	NETLIB_DEVICE_IMPL_NS(analog, QBJT_EB, "QBJT_EB", "MODEL")
	NETLIB_DEVICE_IMPL_NS(analog, QBJT_switch, "QBJT_SW", "MODEL")
} // namespace devices

} // namespace netlist
