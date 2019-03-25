// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_twoterm.h
 *
 * Devices with two terminals ...
 *
 *
 *       (k)
 *  +-----T-----+
 *  |     |     |
 *  |  +--+--+  |
 *  |  |     |  |
 *  |  R     |  |
 *  |  R     |  |
 *  |  R     I  |
 *  |  |     I  |  Device n
 *  |  V+    I  |
 *  |  V     |  |
 *  |  V-    |  |
 *  |  |     |  |
 *  |  +--+--+  |
 *  |     |     |
 *  +-----T-----+
 *       (l)
 *
 *  This is a resistance in series to a voltage source and paralleled by a
 *  current source. This is suitable to model voltage sources, current sources,
 *  resistors, capacitors, inductances and diodes.
 *
 */

#ifndef NLID_TWOTERM_H_
#define NLID_TWOTERM_H_

#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"
#include "netlist/solver/nld_solver.h"
#include "plib/pfunction.h"

#include <cmath>

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------

namespace netlist
{
	namespace analog
	{
// -----------------------------------------------------------------------------
// nld_twoterm
// -----------------------------------------------------------------------------

		template <class C>
		inline core_device_t &bselect(bool b, C &d1, core_device_t &d2)
		{
			auto *h = dynamic_cast<core_device_t *>(&d1);
			return b ? *h : d2;
		}
		template<>
		inline core_device_t &bselect(bool b, netlist_state_t &d1, core_device_t &d2)
		{
			plib::unused_var(d1);
			if (b)
				throw nl_exception("bselect with netlist and b==true");
			return d2;
		}

NETLIB_OBJECT(twoterm)
{
	NETLIB_CONSTRUCTOR_EX(twoterm, bool terminals_owned = false)
	, m_P(bselect(terminals_owned, owner, *this), (terminals_owned ? name + "." : "") + "1", &m_N)
	, m_N(bselect(terminals_owned, owner, *this), (terminals_owned ? name + "." : "") + "2", &m_P)
	{
	}

	terminal_t m_P;
	terminal_t m_N;

	//NETLIB_UPDATE_TERMINALSI() { }
	//NETLIB_RESETI() { }

public:

	NETLIB_UPDATEI();

	void solve_now();

	void solve_later(netlist_time delay = netlist_time::from_nsec(1));

	void set_G_V_I(const nl_double G, const nl_double V, const nl_double I)
	{
		/*      GO, GT, I                */
		m_P.set_go_gt_I( -G,  G, (  V) * G - I);
		m_N.set_go_gt_I( -G,  G, ( -V) * G + I);
	}

	nl_double deltaV() const
	{
		return m_P.net().Q_Analog() - m_N.net().Q_Analog();
	}

	void set_mat(const nl_double a11, const nl_double a12, const nl_double rhs1,
				 const nl_double a21, const nl_double a22, const nl_double rhs2)
	{
		/*      GO, GT, I                */
		m_P.set_go_gt_I(a12, a11, rhs1);
		m_N.set_go_gt_I(a21, a22, rhs2);
	}

private:
};


// -----------------------------------------------------------------------------
// nld_R
// -----------------------------------------------------------------------------

NETLIB_OBJECT_DERIVED(R_base, twoterm)
{
	NETLIB_CONSTRUCTOR_DERIVED(R_base, twoterm)
	{
	}

	void set_R(const nl_double R)
	{
		const nl_double G = plib::constants<nl_double>::one() / R;
		set_mat( G, -G, 0.0,
				-G,  G, 0.0);
	}

	NETLIB_RESETI();

protected:
	//NETLIB_UPDATEI();

};

NETLIB_OBJECT_DERIVED(R, R_base)
{
	NETLIB_CONSTRUCTOR_DERIVED(R, R_base)
	, m_R(*this, "R", 1e9)
	{
	}


protected:

	NETLIB_RESETI();
	//NETLIB_UPDATEI() { }
	NETLIB_UPDATE_PARAMI();

private:
	param_double_t m_R;
	/* protect set_R ... it's a recipe to desaster when used to bypass the parameter */
	using NETLIB_NAME(R_base)::set_R;
};

// -----------------------------------------------------------------------------
// nld_POT
// -----------------------------------------------------------------------------

NETLIB_OBJECT(POT)
{
	NETLIB_CONSTRUCTOR(POT)
	, m_R1(*this, "_R1")
	, m_R2(*this, "_R2")
	, m_R(*this, "R", 10000)
	, m_Dial(*this, "DIAL", 0.5)
	, m_DialIsLog(*this, "DIALLOG", false)
	, m_Reverse(*this, "REVERSE", false)
	{
		register_subalias("1", m_R1.m_P);
		register_subalias("2", m_R1.m_N);
		register_subalias("3", m_R2.m_N);

		connect(m_R2.m_P, m_R1.m_N);

	}

	//NETLIB_UPDATEI();
	NETLIB_RESETI();
	NETLIB_UPDATE_PARAMI();

private:
	NETLIB_SUB(R_base) m_R1;
	NETLIB_SUB(R_base) m_R2;

	param_double_t m_R;
	param_double_t m_Dial;
	param_logic_t m_DialIsLog;
	param_logic_t m_Reverse;
};

NETLIB_OBJECT(POT2)
{
	NETLIB_CONSTRUCTOR(POT2)
	, m_R1(*this, "_R1")
	, m_R(*this, "R", 10000)
	, m_Dial(*this, "DIAL", 0.5)
	, m_DialIsLog(*this, "DIALLOG", false)
	, m_Reverse(*this, "REVERSE", false)
	{
		register_subalias("1", m_R1.m_P);
		register_subalias("2", m_R1.m_N);

	}

	//NETLIB_UPDATEI();
	NETLIB_RESETI();
	NETLIB_UPDATE_PARAMI();

private:
	NETLIB_SUB(R_base) m_R1;

	param_double_t m_R;
	param_double_t m_Dial;
	param_logic_t m_DialIsLog;
	param_logic_t m_Reverse;
};

// -----------------------------------------------------------------------------
// A generic capacitor model
// -----------------------------------------------------------------------------

enum class capacitor_e
{
	CHARGE_CONSERVING,
	CONSTANT_CAPACITY
};

template <capacitor_e TYPE>
class generic_capacitor
{
};


template <>
class generic_capacitor<capacitor_e::CHARGE_CONSERVING>
{
public:
	generic_capacitor(device_t &dev, const pstring &name)
	: m_h(dev, name + ".m_h", 0.0)
	, m_charge(dev, name + ".m_charge", 0.0)
	, m_gmin(0.0)
	{
	}

	constexpr capacitor_e type() const { return capacitor_e::CHARGE_CONSERVING; }

	constexpr nl_double G(nl_double cap) const
	{
		return cap * m_h +  m_gmin;
	}

	constexpr nl_double Ieq(nl_double cap, nl_double v) const
	{
		return m_h * (cap * v - m_charge) - G(cap) * v;
	}

	void timestep(nl_double cap, nl_double v, nl_double step)
	{
		m_h = 1.0 / step;
		m_charge = cap * v;
	}

	void setparams(nl_double gmin) { m_gmin = gmin; }

private:
	state_var<double> m_h;
	state_var<double> m_charge;
	nl_double m_gmin;
};

template <>
class generic_capacitor<capacitor_e::CONSTANT_CAPACITY>
{
public:
	generic_capacitor(device_t &dev, const pstring &name)
	: m_h(dev, name + ".m_h", 0.0)
	, m_gmin(0.0)
	{
	}

	constexpr capacitor_e type() const { return capacitor_e::CONSTANT_CAPACITY; }
	constexpr nl_double G(nl_double cap) const { return cap * m_h +  m_gmin; }
	constexpr nl_double Ieq(nl_double cap, nl_double v) const { return - G(cap) * v; }

	void timestep(nl_double cap, nl_double v, nl_double step)
	{
		plib::unused_var(cap, v);
		m_h = 1.0 / step;
	}
	void setparams(nl_double gmin) { m_gmin = gmin; }
private:
	state_var<nl_double> m_h;
	nl_double m_gmin;
};

// -----------------------------------------------------------------------------
// nld_C
// -----------------------------------------------------------------------------

NETLIB_OBJECT_DERIVED(C, twoterm)
{
public:
	NETLIB_CONSTRUCTOR_DERIVED(C, twoterm)
	, m_C(*this, "C", 1e-6)
	, m_cap(*this, "m_cap")
	{
	}

	NETLIB_IS_TIMESTEP(true)
	NETLIB_TIMESTEPI()
	{
		m_cap.timestep(m_C(), deltaV(), step);
		if (m_cap.type() == capacitor_e::CONSTANT_CAPACITY)
		{
			const nl_double I = m_cap.Ieq(m_C(), deltaV());
			const nl_double G = m_cap.G(m_C());
			set_mat( G, -G, -I,
					-G,  G,  I);
		}
	}

	NETLIB_IS_DYNAMIC(m_cap.type() == capacitor_e::CHARGE_CONSERVING)
	NETLIB_UPDATE_TERMINALSI()
	{
		const nl_double I = m_cap.Ieq(m_C(), deltaV());
		const nl_double G = m_cap.G(m_C());
		set_mat( G, -G, -I,
				-G,  G,  I);
	}

	param_double_t m_C;
	NETLIB_RESETI()
	{
		m_cap.setparams(exec().gmin());
	}

protected:
	//NETLIB_UPDATEI();
	NETLIB_UPDATE_PARAMI() { }

private:
	generic_capacitor<capacitor_e::CHARGE_CONSERVING> m_cap;
	//generic_capacitor<capacitor_e::CONSTANT_CAPACITY> m_cap;
};

// -----------------------------------------------------------------------------
// nld_L
// -----------------------------------------------------------------------------

NETLIB_OBJECT_DERIVED(L, twoterm)
{
public:
	NETLIB_CONSTRUCTOR_DERIVED(L, twoterm)
	, m_L(*this, "L", 1e-6)
	, m_gmin(0.0)
	, m_G(0.0)
	, m_I(0.0)
	{
		//register_term("1", m_P);
		//register_term("2", m_N);
	}

	NETLIB_IS_TIMESTEP(true)
	NETLIB_TIMESTEPI();
	NETLIB_RESETI();

protected:
	//NETLIB_UPDATEI();
	NETLIB_UPDATE_PARAMI();

private:
	param_double_t m_L;

	nl_double m_gmin;
	nl_double m_G;
	nl_double m_I;
};

// -----------------------------------------------------------------------------
// A generic diode model to be used in other devices (Diode, BJT ...)
// -----------------------------------------------------------------------------

enum class diode_e
{
	BIPOLAR,
	MOS
};

template <diode_e TYPE>
class generic_diode
{
public:
	generic_diode(device_t &dev, const pstring &name)
	: m_Vd(dev, name + ".m_Vd", 0.7)
	, m_Id(dev, name + ".m_Id", 0.0)
	, m_G(dev,  name + ".m_G", 1e-15)
	, m_Vt(0.0)
	, m_Vmin(0.0) // not used in MOS model
	, m_Is(0.0)
	, m_logIs(0.0)
	, m_n(0.0)
	, m_gmin(1e-15)
	, m_VtInv(0.0)
	, m_Vcrit(0.0)
	{
		set_param(1e-15, 1, 1e-15, 300.0);
	}

	void update_diode(const double nVd)
	{
		if (TYPE == diode_e::BIPOLAR && nVd < m_Vmin)
		{
			m_Vd = nVd;
			m_G = m_gmin;
			m_Id = - m_Is;
		}
		else if (TYPE == diode_e::MOS && nVd < constants::zero())
		{
			m_Vd = nVd;
			m_G = m_Is * m_VtInv + m_gmin;
			m_Id = m_G * m_Vd;
		}
		// FIXME: For MOS, stop here, the critical code path will not converge
		else if (TYPE == diode_e::MOS || nVd < m_Vcrit)
		{
			m_Vd = nVd;
			const double IseVDVt = std::exp(std::min(700.0, m_logIs + m_Vd * m_VtInv));
			m_Id = IseVDVt - m_Is;
			m_G = IseVDVt * m_VtInv + m_gmin;
		}
		else
		{
			const double a = std::max((nVd - m_Vd) * m_VtInv, constants::cast(-0.99));
			m_Vd = m_Vd + std::log1p(a) * m_Vt;
			//const double IseVDVt = m_Is * std::exp(m_Vd * m_VtInv);
			const double IseVDVt = std::exp(m_logIs + m_Vd * m_VtInv);
			m_Id = IseVDVt - m_Is;
			m_G = IseVDVt * m_VtInv + m_gmin;
		}
	}


	void set_param(const double Is, const double n, double gmin, double temp)
	{
		static constexpr double csqrt2 = 1.414213562373095048801688724209; //std::sqrt(2.0);
		m_Is = Is;
		m_logIs = std::log(Is);
		m_n = n;
		m_gmin = gmin;

		m_Vt = m_n * temp * constants::k_b() / constants::Q_e();

		m_Vmin = -5.0 * m_Vt;

		m_Vcrit = m_Vt * std::log(m_Vt / m_Is / csqrt2);
		m_VtInv = 1.0 / m_Vt;
	}


	double I() const { return m_Id; }
	double G() const { return m_G; }
	double Ieq() const { return (m_Id - m_Vd * m_G); }
	double Vd() const { return m_Vd; }

	/* owning object must save those ... */

private:
	state_var<double> m_Vd;
	state_var<double> m_Id;
	state_var<double> m_G;

	double m_Vt;
	double m_Vmin;
	double m_Is;
	double m_logIs;
	double m_n;
	double m_gmin;

	double m_VtInv;
	double m_Vcrit;
};

/*! Class representing the diode model paramers.
 *  This is the model representation of the diode model. Typically, SPICE uses
 *  the following parameters. A "Y" in the first column indicates that the
 *  parameter is actually used in netlist.
 *
 *   |NL? |name  |parameter                        |units|default| example|area  |
 *   |:--:|:-----|:--------------------------------|:----|------:|-------:|:----:|
 *   | Y  |IS    |saturation current               |A    |1.0e-14| 1.0e-14|   *  |
 *   |    |RS    |ohmic resistanc                  |Ohm  |      0|      10|   *  |
 *   | Y  |N     |emission coefficient             |-    |      1|       1|      |
 *   |    |TT    |transit-time                     |sec  |      0|   0.1ns|      |
 *   |    |CJO   |zero-bias junction capacitance   |F    |      0|     2pF|   *  |
 *   |    |VJ    |junction potential               |V    |      1|     0.6|      |
 *   |    |M     |grading coefficient              |-    |    0.5|     0.5|      |
 *   |    |EG    |band-gap energy                  |eV   |   1.11| 1.11 Si|      |
 *   |    |XTI   |saturation-current temp.exp      |-    |      3|3.0 pn. 2.0 Schottky| |
 *   |    |KF    |flicker noise coefficient        |-    |      0|        |      |
 *   |    |AF    |flicker noise exponent           |-    |      1|        |      |
 *   |    |FC    |coefficient for forward-bias depletion capacitance formula|-|0.5|| |
 *   |    |BV    |reverse breakdown voltage        |V    |infinite|     40|      |
 *   |    |IBV   |current at breakdown voltage     |V    |  0.001|        |      |
 *   |    |TNOM  |parameter measurement temperature|deg C|     27|      50|      |
 *
 */

class diode_model_t : public param_model_t
{
public:
	diode_model_t(device_t &device, const pstring &name, const pstring &val)
	: param_model_t(device, name, val)
	, m_IS(*this, "IS")
	, m_N(*this, "N")
	{}

	value_t m_IS;    //!< saturation current.
	value_t m_N;     //!< emission coefficient.
};


// -----------------------------------------------------------------------------
// nld_D
// -----------------------------------------------------------------------------

NETLIB_OBJECT_DERIVED(D, twoterm)
{
public:
	NETLIB_CONSTRUCTOR_DERIVED(D, twoterm)
	, m_model(*this, "MODEL", "D")
	, m_D(*this, "m_D")
	{
		register_subalias("A", m_P);
		register_subalias("K", m_N);
	}

	template <class CLASS>
	NETLIB_NAME(D)(CLASS &owner, const pstring &name, const pstring &model)
	: NETLIB_NAME(twoterm)(owner, name)
	, m_model(*this, "MODEL", model)
	, m_D(*this, "m_D")
	{
		register_subalias("A", m_P);
		register_subalias("K", m_N);
	}

	NETLIB_IS_DYNAMIC(true)
	NETLIB_UPDATE_TERMINALSI();
	NETLIB_RESETI();

protected:
	//NETLIB_UPDATEI();
	NETLIB_UPDATE_PARAMI();

private:
	diode_model_t m_model;
	generic_diode<diode_e::BIPOLAR> m_D;
};


// -----------------------------------------------------------------------------
// nld_VS - Voltage source
//
// netlist voltage source must have inner resistance
// -----------------------------------------------------------------------------

NETLIB_OBJECT_DERIVED(VS, twoterm)
{
public:
	NETLIB_CONSTRUCTOR_DERIVED(VS, twoterm)
	, m_t(*this, "m_t", 0.0)
	, m_R(*this, "R", 0.1)
	, m_V(*this, "V", 0.0)
	, m_func(*this,"FUNC", "")
	, m_compiled(this->name() + ".FUNCC", this, this->state().run_state_manager())
	{
		register_subalias("P", m_P);
		register_subalias("N", m_N);
		if (m_func() != "")
			m_compiled.compile_postfix(std::vector<pstring>({{"T"}}), m_func());
	}

	NETLIB_IS_TIMESTEP(m_func() != "")

	NETLIB_TIMESTEPI()
	{
		m_t += step;
		this->set_G_V_I(1.0 / m_R(),
				m_compiled.evaluate(std::vector<double>({m_t})),
				0.0);
	}

protected:
	// NETLIB_UPDATEI() {   NETLIB_NAME(twoterm)::update(time); }

	NETLIB_RESETI()
	{
		NETLIB_NAME(twoterm)::reset();
		this->set_G_V_I(1.0 / m_R(), m_V(), 0.0);
	}

private:
	state_var<double> m_t;
	param_double_t m_R;
	param_double_t m_V;
	param_str_t m_func;
	plib::pfunction m_compiled;
};

// -----------------------------------------------------------------------------
// nld_CS - Current source
// -----------------------------------------------------------------------------

NETLIB_OBJECT_DERIVED(CS, twoterm)
{
public:
	NETLIB_CONSTRUCTOR_DERIVED(CS, twoterm)
	, m_t(*this, "m_t", 0.0)
	, m_I(*this, "I", 1.0)
	, m_func(*this,"FUNC", "")
	, m_compiled(this->name() + ".FUNCC", this, this->state().run_state_manager())
	{
		register_subalias("P", m_P);
		register_subalias("N", m_N);
		if (m_func() != "")
			m_compiled.compile_postfix(std::vector<pstring>({{"T"}}), m_func());
	}

	NETLIB_IS_TIMESTEP(m_func() != "")
	NETLIB_TIMESTEPI()
	{
		m_t += step;
		const double I = m_compiled.evaluate(std::vector<double>({m_t}));
		set_mat(0.0, 0.0, -I,
				0.0, 0.0,  I);
	}

protected:

	//NETLIB_UPDATEI() { NETLIB_NAME(twoterm)::update(time); }
	NETLIB_RESETI()
	{
		NETLIB_NAME(twoterm)::reset();
		set_mat(0.0, 0.0, -m_I(),
				0.0, 0.0,  m_I());
	}

private:
	state_var<double> m_t;
	param_double_t m_I;
	param_str_t m_func;
	plib::pfunction m_compiled;
};


	} // namespace analog
} // namespace netlist

#endif /* NLD_TWOTERM_H_ */
