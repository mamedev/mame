// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_bjt.c
 *
 */

#include "../solver/nld_solver.h"
#include "../nl_setup.h"
#include "nlid_twoterm.h"

#include <cmath>

namespace netlist
{
	namespace analog
	{

class diode
{
public:
	diode() : m_Is(1e-15), m_VT(0.0258), m_VT_inv(1.0 / m_VT) {}
	diode(const nl_double Is, const nl_double n)
	{
		m_Is = Is;
		m_VT = 0.0258 * n;
		m_VT_inv = 1.0 / m_VT;
	}
	void set(const nl_double Is, const nl_double n)
	{
		m_Is = Is;
		m_VT = 0.0258 * n;
		m_VT_inv = 1.0 / m_VT;
	}
	nl_double I(const nl_double V) const { return m_Is * std::exp(V * m_VT_inv) - m_Is; }
	nl_double g(const nl_double V) const { return m_Is * m_VT_inv * std::exp(V * m_VT_inv); }
	nl_double V(const nl_double I) const { return std::log1p(I / m_Is) * m_VT; } // log1p(x)=log(1.0 + x)
	nl_double gI(const nl_double I) const { return m_VT_inv * (I + m_Is); }

private:
	nl_double m_Is;
	nl_double m_VT;
	nl_double m_VT_inv;
};

// -----------------------------------------------------------------------------
// nld_Q - Base classes
// -----------------------------------------------------------------------------

	/*! Class representing the bjt model paramers.
	 *
	 *  This is the model representation of the bjt model. Typically, SPICE uses
	 *  the following parameters. A "Y" in the first column indicates that the
	 *  parameter is actually used in netlist.
	 *
	 * | NL? | name | parameter                                                             | units |  default |         example | area |
	 * |:---:|------|-----------------------------------------------------------------------|-------|---------:|----------------:|:----:|
	 * |  Y  | IS   | transport saturation current                                          | A     |   1E-016 |          1E-015 |   *  |
	 * |  Y  | BF   | ideal maximum forward beta                                            | -     |      100 |             100 |      |
	 * |  Y  | NF   | forward current emission coefficient                                  | -     |        1 |               1 |      |
	 * |     | VAF  | forward Early voltage                                                 | V     | infinite |             200 |      |
	 * |     | IKF  | corner for forward beta high current roll-off                         | A     | infinite |            0.01 |   *  |
	 * |     | ISE  | B-E leakage saturation current                                        | A     |        0 | 0.0000000000001 |   *  |
	 * |     | NE   | B-E leakage emission coefficient                                      | -     |      1.5 |               2 |      |
	 * |  Y  | BR   | ideal maximum reverse beta                                            | -     |        1 |             0.1 |      |
	 * |  Y  | NR   | reverse current emission coefficient                                  | -     |        1 |               1 |      |
	 * |     | VAR  | reverse Early voltage                                                 | V     | infinite |             200 |      |
	 * |     | IKR  | corner for reverse beta high current roll-off                         | A     | infinite |            0.01 |   *  |
	 * |     | ISC  | leakage saturation current                                            | A     |        0 |               8 |      |
	 * |     | NC   | leakage emission coefficient                                          | -     |        2 |             1.5 |      |
	 * |     | RB   | zero bias base resistance                                             |       |        0 |             100 |   *  |
	 * |     | IRB  | current where base resistance falls halfway to its min value          | A     |  infinte |             0.1 |   *  |
	 * |     | RBM  | minimum base resistance at high currents                              |       |       RB |              10 |   *  |
	 * |     | RE   | emitter resistance                                                    |       |        0 |               1 |   *  |
	 * |     | RC   | collector resistance                                                  |       |        0 |              10 |   *  |
	 * |     | CJE  | B-E zero-bias depletion capacitance                                   | F     |        0 |             2pF |   *  |
	 * |     | VJE  | B-E built-in potential                                                | V     |     0.75 |             0.6 |      |
	 * |     | MJE  | B-E junction exponential factor                                       | -     |     0.33 |            0.33 |      |
	 * |     | TF   | ideal forward transit time                                            | sec   |        0 |           0.1ns |      |
	 * |     | XTF  | coefficient for bias dependence of TF                                 | -     |        0 |                 |      |
	 * |     | VTF  | voltage describing VBC  dependence of TF                              | V     | infinite |                 |      |
	 * |     | ITF  | high-current parameter  for effect on TF                              | A     |        0 |                 |   *  |
	 * |     | PTF  | excess phase at freq=1.0/(TF*2PI) Hz                                  | deg   |        0 |                 |      |
	 * |     | CJC  | B-C zero-bias depletion capacitance                                   | F     |        0 |             2pF |   *  |
	 * |     | VJC  | B-C built-in potential                                                | V     |     0.75 |             0.5 |      |
	 * |     | MJC  | B-C junction exponential factor                                       | -     |     0.33 |             0.5 |      |
	 * |     | XCJC | fraction of B-C depletion capacitance connected to internal base node | -     |        1 |                 |      |
	 * |     | TR   | ideal reverse transit time                                            | sec   |        0 |            10ns |      |
	 * |     | CJS  | zero-bias collector-substrate capacitance                             | F     |        0 |             2pF |   *  |
	 * |     | VJS  | substrate junction built-in potential                                 | V     |     0.75 |                 |      |
	 * |     | MJS  | substrate junction exponential factor                                 | -     |        0 |             0.5 |      |
	 * |     | XTB  | forward and reverse beta temperature exponent                         | -     |        0 |                 |      |
	 * |     | EG   | energy gap for temperature effect on IS                               | eV    |     1.11 |                 |      |
	 * |     | XTI  | temperature exponent for effect on IS                                 | -     |        3 |                 |      |
	 * |     | KF   | flicker-noise coefficient                                             | -     |        0 |                 |      |
	 * |     | AF   | flicker-noise exponent                                                | -     |        1 |                 |      |
	 * |     | FC   | coefficient for forward-bias depletion capacitance formula            | -     |      0.5 |                 |      |
	 * |     | TNOM | Parameter measurement temperature                                     | C     |       27 |              50 |      |    */

	class bjt_model_t : public param_model_t
	{
	public:
		bjt_model_t(device_t &device, const pstring &name, const pstring &val)
		: param_model_t(device, name, val)
		, m_IS(*this, "IS")
		, m_BF(*this, "BF")
		, m_NF(*this, "NF")
		, m_BR(*this, "BR")
		, m_NR(*this, "NR")
		{}

		value_t m_IS; //!< transport saturation current
		value_t m_BF; //!< ideal maximum forward beta
		value_t m_NF; //!< forward current emission coefficient
		value_t m_BR; //!< ideal maximum reverse beta
		value_t m_NR; //!< reverse current emission coefficient
	};

	// Have a common start for transistors

NETLIB_OBJECT(Q)
{
public:
	enum q_type {
		BJT_NPN,
		BJT_PNP
	};

	NETLIB_CONSTRUCTOR(Q)
	, m_model(*this, "MODEL", "NPN")
	, m_qtype(BJT_NPN)
	{
	}

	NETLIB_IS_DYNAMIC(true)

	//NETLIB_RESETI();
	NETLIB_UPDATEI();

	q_type qtype() const { return m_qtype; }
	bool is_qtype(q_type atype) const { return m_qtype == atype; }
	void set_qtype(q_type atype) { m_qtype = atype; }
protected:

	bjt_model_t m_model;
private:
	q_type m_qtype;
};

NETLIB_OBJECT_DERIVED(QBJT, Q)
{
public:
	NETLIB_CONSTRUCTOR_DERIVED(QBJT, Q)
		{ }

protected:

private:
};




// -----------------------------------------------------------------------------
// nld_QBJT_switch
// -----------------------------------------------------------------------------


/*
 *         + -              C
 *   B ----VVV----+         |
 *                |         |
 *                Rb        Rc
 *                Rb        Rc
 *                Rb        Rc
 *                |         |
 *                +----+----+
 *                     |
 *                     E
 */

NETLIB_OBJECT_DERIVED(QBJT_switch, QBJT)
{
	NETLIB_CONSTRUCTOR_DERIVED(QBJT_switch, QBJT)
		, m_RB(*this, "m_RB", true)
		, m_RC(*this, "m_RC", true)
		, m_BC_dummy(*this, "m_BC", true)
		, m_gB(1e-9)
		, m_gC(1e-9)
		, m_V(0.0)
		, m_state_on(*this, "m_state_on", 0)
	{
		register_subalias("B", m_RB.m_P);
		register_subalias("E", m_RB.m_N);
		register_subalias("C", m_RC.m_P);
		//register_term("_E1", m_RC.m_N);

		//register_term("_B1", m_BC_dummy.m_P);
		//register_term("_C1", m_BC_dummy.m_N);

		connect(m_RB.m_N, m_RC.m_N);

		connect(m_RB.m_P, m_BC_dummy.m_P);
		connect(m_RC.m_P, m_BC_dummy.m_N);
	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();
	NETLIB_UPDATE_PARAMI();
	NETLIB_UPDATE_TERMINALSI();

	nld_twoterm m_RB;
	nld_twoterm m_RC;

	// FIXME: this is needed so we have all terminals belong to one net list

	nld_twoterm m_BC_dummy;

protected:


	nl_double m_gB; // base conductance / switch on
	nl_double m_gC; // collector conductance / switch on
	nl_double m_V; // internal voltage source
	state_var<unsigned> m_state_on;

private:
};

// -----------------------------------------------------------------------------
// nld_QBJT_EB
// -----------------------------------------------------------------------------


NETLIB_OBJECT_DERIVED(QBJT_EB, QBJT)
{
public:
	NETLIB_CONSTRUCTOR_DERIVED(QBJT_EB, QBJT)
	, m_gD_BC(*this, "m_D_BC")
	, m_gD_BE(*this, "m_D_BE")
	, m_D_CB(*this, "m_D_CB", true)
	, m_D_EB(*this, "m_D_EB", true)
	, m_D_EC(*this, "m_D_EC", true)
	, m_alpha_f(0)
	, m_alpha_r(0)
	{
		register_subalias("E", m_D_EB.m_P);   // Cathode
		register_subalias("B", m_D_EB.m_N);   // Anode

		register_subalias("C", m_D_CB.m_P);   // Cathode

		connect(m_D_EB.m_P, m_D_EC.m_P);
		connect(m_D_EB.m_N, m_D_CB.m_N);
		connect(m_D_CB.m_P, m_D_EC.m_N);
	}

protected:

	NETLIB_RESETI();
	NETLIB_UPDATEI();
	NETLIB_UPDATE_PARAMI();
	NETLIB_UPDATE_TERMINALSI();

	generic_diode m_gD_BC;
	generic_diode m_gD_BE;

private:
	nld_twoterm m_D_CB;  // gcc, gce - gcc, gec - gcc, gcc - gce | Ic
	nld_twoterm m_D_EB;  // gee, gec - gee, gce - gee, gee - gec | Ie
	nld_twoterm m_D_EC;  // 0, -gec, -gcc, 0 | 0

	nl_double m_alpha_f;
	nl_double m_alpha_r;

};


// ----------------------------------------------------------------------------------------
// nld_Q
// ----------------------------------------------------------------------------------------

NETLIB_UPDATE(Q)
{
//    netlist().solver()->schedule1();
}

// ----------------------------------------------------------------------------------------
// nld_QBJT_switch
// ----------------------------------------------------------------------------------------


NETLIB_RESET(QBJT_switch)
{
	NETLIB_NAME(Q)::reset();

	m_state_on = 0;

	m_RB.set(exec().gmin(), 0.0, 0.0);
	m_RC.set(exec().gmin(), 0.0, 0.0);

	m_BC_dummy.set(exec().gmin() / 10.0, 0.0, 0.0);

}

NETLIB_UPDATE(QBJT_switch)
{
	if (!m_RB.m_P.net().isRailNet())
		m_RB.m_P.solve_now();   // Basis
	else if (!m_RB.m_N.net().isRailNet())
		m_RB.m_N.solve_now();   // Emitter
	else if (!m_RC.m_P.net().isRailNet())
		m_RC.m_P.solve_now();   // Collector
}


NETLIB_UPDATE_PARAM(QBJT_switch)
{
	nl_double IS = m_model.m_IS;
	nl_double BF = m_model.m_BF;
	nl_double NF = m_model.m_NF;
	//nl_double VJE = m_model.dValue("VJE", 0.75);

	set_qtype((m_model.model_type() == "NPN") ? BJT_NPN : BJT_PNP);

	nl_double alpha = BF / (1.0 + BF);

	diode d(IS, NF);

	// Assume 5mA Collector current for switch operation

	m_V = d.V(0.005 / alpha);

	/* Base current is 0.005 / beta
	 * as a rough estimate, we just scale the conductance down */

	m_gB = 1.0 / (m_V/(0.005 / BF));

	//m_gB = d.gI(0.005 / alpha);

	if (m_gB < exec().gmin())
		m_gB = exec().gmin();
	m_gC =  d.gI(0.005); // very rough estimate
}

NETLIB_UPDATE_TERMINALS(QBJT_switch)
{
	const nl_double m = (is_qtype( BJT_NPN) ? 1 : -1);

	const unsigned new_state = (m_RB.deltaV() * m > m_V ) ? 1 : 0;
	if (m_state_on ^ new_state)
	{
		const nl_double gb = new_state ? m_gB : exec().gmin();
		const nl_double gc = new_state ? m_gC : exec().gmin();
		const nl_double v  = new_state ? m_V * m : 0;

		m_RB.set(gb, v,   0.0);
		m_RC.set(gc, 0.0, 0.0);
		m_state_on = new_state;
	}
}


// ----------------------------------------------------------------------------------------
// nld_Q - Ebers Moll
// ----------------------------------------------------------------------------------------


NETLIB_UPDATE(QBJT_EB)
{
	if (!m_D_EB.m_P.net().isRailNet())
		m_D_EB.m_P.solve_now();   // Basis
	else if (!m_D_EB.m_N.net().isRailNet())
		m_D_EB.m_N.solve_now();   // Emitter
	else
		m_D_CB.m_N.solve_now();   // Collector
}

NETLIB_RESET(QBJT_EB)
{
	NETLIB_NAME(Q)::reset();
}

NETLIB_UPDATE_TERMINALS(QBJT_EB)
{
	const nl_double polarity = (qtype() == BJT_NPN ? 1.0 : -1.0);

	m_gD_BE.update_diode(-m_D_EB.deltaV() * polarity);
	m_gD_BC.update_diode(-m_D_CB.deltaV() * polarity);

	const nl_double gee = m_gD_BE.G();
	const nl_double gcc = m_gD_BC.G();
	const nl_double gec =  m_alpha_r * gcc;
	const nl_double gce =  m_alpha_f * gee;
	const nl_double sIe = -m_gD_BE.I() + m_alpha_r * m_gD_BC.I();
	const nl_double sIc = m_alpha_f * m_gD_BE.I() - m_gD_BC.I();
	const nl_double Ie = (sIe + gee * m_gD_BE.Vd() - gec * m_gD_BC.Vd()) * polarity;
	const nl_double Ic = (sIc - gce * m_gD_BE.Vd() + gcc * m_gD_BC.Vd()) * polarity;

	m_D_EB.set_mat(      gee, gec - gee,  -Ie,
				   gce - gee, gee - gec,   Ie);
	m_D_CB.set_mat(      gcc, gce - gcc,  -Ic,
				   gec - gcc, gcc - gce,   Ic);
	m_D_EC.set_mat(        0,      -gec,    0,
						-gce,         0,    0);
}


NETLIB_UPDATE_PARAM(QBJT_EB)
{
	nl_double IS = m_model.m_IS;
	nl_double BF = m_model.m_BF;
	nl_double NF = m_model.m_NF;
	nl_double BR = m_model.m_BR;
	nl_double NR = m_model.m_NR;
	//nl_double VJE = m_model.dValue("VJE", 0.75);

	set_qtype((m_model.model_type() == "NPN") ? BJT_NPN : BJT_PNP);

	m_alpha_f = BF / (1.0 + BF);
	m_alpha_r = BR / (1.0 + BR);

	m_gD_BE.set_param(IS / m_alpha_f, NF, exec().gmin());
	m_gD_BC.set_param(IS / m_alpha_r, NR, exec().gmin());
}

	} //namespace analog

	namespace devices {
		NETLIB_DEVICE_IMPL_NS(analog, QBJT_EB, "QBJT_EB", "MODEL")
		NETLIB_DEVICE_IMPL_NS(analog, QBJT_switch, "QBJT_SW", "MODEL")
	} // namespace devices

} // namespace netlist
