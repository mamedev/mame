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
 *  This is a resistance in series to a voltage source and paralleled by a current source.
 *  This is suitable to model voltage sources, current sources, resistors, capacitors,
 *  inductances and diodes.
 *
 */

#ifndef NLD_TWOTERM_H_
#define NLD_TWOTERM_H_

#include "../nl_base.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define NETDEV_R(_name, _R)                                                         \
		NET_REGISTER_DEV(R, _name)                                                  \
		NETDEV_PARAMI(_name, R, _R)

#define NETDEV_POT(_name, _R)                                                       \
		NET_REGISTER_DEV(POT, _name)                                                \
		NETDEV_PARAMI(_name, R, _R)


#define NETDEV_C(_name, _C)                                                         \
		NET_REGISTER_DEV(C, _name)                                                  \
		NETDEV_PARAMI(_name, C, _C)

/* Generic Diode */
#define NETDEV_D(_name,  _model)                                                    \
		NET_REGISTER_DEV(D, _name)                                                  \
		NETDEV_PARAMI(_name, model, # _model)

#define NETDEV_QPNP(_name, _model)                                                 \
		NET_REGISTER_DEV(QPNP_switch, _name)                                       \
		NETDEV_PARAMI(_name,  model, # _model)

#define NETDEV_QNPN(_name, _model)                                                 \
		NET_REGISTER_DEV(QNPN_switch, _name)                                       \
		NETDEV_PARAMI(_name,  model, # _model)

// ----------------------------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------
// nld_twoterm
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(twoterm) : public netlist_device_t
{
public:
	ATTR_COLD NETLIB_NAME(twoterm)(const family_t afamily);

	netlist_terminal_t m_P;
	netlist_terminal_t m_N;

	virtual NETLIB_UPDATE_TERMINALS()
	{
	}

	ATTR_HOT inline void set(const double G, const double V, const double I)
	{
		m_P.m_go = m_N.m_go = m_P.m_gt = m_N.m_gt = G;
		m_N.m_Idr = ( -V) * G + I;
		m_P.m_Idr = (  V) * G - I;
	}
protected:
	ATTR_COLD virtual void start();
	ATTR_HOT ATTR_ALIGN void update();

private:
};

// ----------------------------------------------------------------------------------------
// nld_R
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(R_base) : public NETLIB_NAME(twoterm)
{
public:
	ATTR_COLD NETLIB_NAME(R_base)() : NETLIB_NAME(twoterm)(RESISTOR) { }

	inline void set_R(const double R) { set(1.0 / R, 0.0, 0.0); }

protected:
	ATTR_COLD virtual void start();
	ATTR_HOT ATTR_ALIGN void update();
};

NETLIB_DEVICE_WITH_PARAMS_DERIVED(R, R_base,
	netlist_param_double_t m_R;
);

// ----------------------------------------------------------------------------------------
// nld_POT
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(POT,
	NETLIB_NAME(R_base) m_R1;
	NETLIB_NAME(R_base) m_R2;

	netlist_param_double_t m_R;
	netlist_param_double_t m_Dial;
	netlist_param_logic_t m_DialIsLog;
);


// ----------------------------------------------------------------------------------------
// nld_C
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(C) : public NETLIB_NAME(twoterm)
{
public:
	ATTR_COLD NETLIB_NAME(C)() : NETLIB_NAME(twoterm)(CAPACITOR) { }

	ATTR_HOT void step_time(const double st)
	{
		double G = m_C.Value() / st;
		double I = -G * (m_P.net().Q_Analog()- m_N.net().Q_Analog());
		set(G, 0.0, I);
	}

protected:
	ATTR_COLD virtual void start();
	ATTR_COLD virtual void update_param();
	ATTR_HOT ATTR_ALIGN void update();

	netlist_param_double_t m_C;

};

// ----------------------------------------------------------------------------------------
// nld_D
// ----------------------------------------------------------------------------------------


// this one has an accuracy of better than 5%. That's enough for our purpose
// add c3 and it'll be better than 1%

#if 0
inline double fastexp_h(const double x)
{
	static const double ln2r = 1.442695040888963387;
	static const double ln2  = 0.693147180559945286;
	//static const double c3   = 0.166666666666666667;

	const double y = x * ln2r;
	const unsigned int t = y;
	const double z = (x - ln2 * (double) t);
	const double zz = z * z;
	//const double zzz = zz * z;

	return (double)(1 << t)*(1.0 + z + 0.5 * zz); // + c3*zzz;
}

inline double fastexp(const double x)
{
	if (x<0)
		return 1.0 / fastexp_h(-x);
	else
		return fastexp_h(x);
}
#endif

class NETLIB_NAME(D) : public NETLIB_NAME(twoterm)
{
public:
	ATTR_COLD NETLIB_NAME(D)() : NETLIB_NAME(twoterm)(DIODE) { }

	NETLIB_UPDATE_TERMINALS()
	{
		const double nVd = m_P.net().Q_Analog()- m_N.net().Q_Analog();

		//FIXME: Optimize cutoff case

		double Id;
		double G;

		if (nVd < -5.0 * m_Vt)
		{
			m_Vd = nVd;
			G = NETLIST_GMIN;
			Id = - m_Is;
		}
		else if (nVd < m_Vcrit)
		{
			m_Vd = nVd;

			const double eVDVt = exp(m_Vd * m_VtInv);
			Id = m_Is * (eVDVt - 1.0);
			G = m_Is * m_VtInv * eVDVt;
		}
		else
		{
#if defined(_MSC_VER) && _MSC_VER < 1800
			m_Vd = m_Vd + log((nVd - m_Vd) * m_VtInv + 1.0) * m_Vt;
#else
			m_Vd = m_Vd + log1p((nVd - m_Vd) * m_VtInv) * m_Vt;
#endif
			const double eVDVt = exp(m_Vd * m_VtInv);
			Id = m_Is * (eVDVt - 1.0);

			G = m_Is * m_VtInv * eVDVt;
		}

		double I = (Id - m_Vd * G);
		set(G, 0.0, I);
	}

protected:
	ATTR_COLD virtual void start();
	ATTR_COLD virtual void update_param();
	ATTR_HOT ATTR_ALIGN void update();

	netlist_param_model_t m_model;

	double m_Vt;
	double m_Is;
	double m_n;

	double m_VtInv;
	double m_Vcrit;
	double m_Vd;

};

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

// Have a common start for transistors

class NETLIB_NAME(Q) : public netlist_device_t
{
public:
	enum q_type {
		BJT_NPN,
		BJT_PNP
	};

	ATTR_COLD NETLIB_NAME(Q)(const q_type atype, const family_t afamily)
	: netlist_device_t(afamily)
	, m_qtype(atype) { }

	inline q_type qtype() const { return m_qtype; }
	inline bool is_qtype(q_type atype) const { return m_qtype == atype; }
protected:
	ATTR_COLD virtual void start();
	ATTR_HOT ATTR_ALIGN void update();

	netlist_param_model_t m_model;
private:
	q_type m_qtype;
};

class NETLIB_NAME(QBJT) : public NETLIB_NAME(Q)
{
public:

	ATTR_COLD NETLIB_NAME(QBJT)(const q_type atype, const family_t afamily)
	: NETLIB_NAME(Q)(atype, afamily) { }

protected:

private:
};

//NETLIB_NAME(Q) nld_Q::q_type
template <NETLIB_NAME(Q)::q_type _type>
class NETLIB_NAME(QBJT_switch) : public NETLIB_NAME(QBJT)
{
public:
	ATTR_COLD NETLIB_NAME(QBJT_switch)()
	: NETLIB_NAME(QBJT)(_type, BJT_SWITCH), m_gB(NETLIST_GMIN), m_gC(NETLIST_GMIN), m_V(0.0), m_state_on(0) { }

	NETLIB_UPDATEI()
	{
		double vE = INPANALOG(m_EV);
		double vB = INPANALOG(m_BV);
		double m = (_type == BJT_NPN) ? 1 : -1;

		int new_state = ((vB - vE) * m > m_V ) ? 1 : 0;
		if (m_state_on ^ new_state)
		{
			double gb = m_gB;
			double gc = m_gC;
			double v  = m_V * m;
			if (!new_state )
			{
				// not conducting
				gb = NETLIST_GMIN;
				v = 0;
				gc = NETLIST_GMIN;
			}
			m_RB.set(gb, v,   0.0);
			m_RC.set(gc, 0.0, 0.0);
			m_state_on = new_state;
			m_RB.update_dev();
			m_RC.update_dev();
		}

	}

	NETLIB_NAME(R) m_RB;
	NETLIB_NAME(R) m_RC;

	netlist_analog_input_t m_BV;
	netlist_analog_input_t m_EV;

protected:

	ATTR_COLD virtual void start();
	ATTR_HOT void update_param();

	double m_gB; // base conductance / switch on
	double m_gC; // collector conductance / switch on
	double m_V; // internal voltage source
	UINT8 m_state_on;

private:
};

typedef NETLIB_NAME(QBJT_switch)<NETLIB_NAME(Q)::BJT_PNP> NETLIB_NAME(QPNP_switch);
typedef NETLIB_NAME(QBJT_switch)<NETLIB_NAME(Q)::BJT_NPN> NETLIB_NAME(QNPN_switch);

// ----------------------------------------------------------------------------------------
// nld_VCCS
// ----------------------------------------------------------------------------------------

/*
 *   Voltage controlled current source
 *
 *   IP ---+           +------> OP
 *         |           |
 *         RI          I
 *         RI => G =>  I    IOut = (V(IP)-V(IN)) * G
 *         RI          I
 *         |           |
 *   IN ---+           +------< ON
 *
 *   G=1 ==> 1V ==> 1A
 *
 *   RI = 1 / NETLIST_GMIN
 *
 */

#define NETDEV_VCCS(_name)                                                         \
		NET_REGISTER_DEV(VCCS, _name)
//NETDEV_PARAMI(_name, model, _model)

class NETLIB_NAME(VCCS) : public netlist_device_t
{
public:
	ATTR_COLD NETLIB_NAME(VCCS)()
	: netlist_device_t(VCCS) {  }
	ATTR_COLD NETLIB_NAME(VCCS)(const family_t afamily)
	: netlist_device_t(afamily) {  }

protected:
	ATTR_COLD virtual void start();
	ATTR_COLD virtual void update_param();
	ATTR_HOT ATTR_ALIGN void update();

	ATTR_COLD void configure(const double Gfac, const double GI);

	netlist_terminal_t m_OP;
	netlist_terminal_t m_ON;

	netlist_terminal_t m_IP;
	netlist_terminal_t m_IN;

	netlist_terminal_t m_OP1;
	netlist_terminal_t m_ON1;

	netlist_param_double_t m_G;
};

// ----------------------------------------------------------------------------------------
// nld_VCVS
// ----------------------------------------------------------------------------------------

/*
 *   Voltage controlled voltage source
 *
 *   Parameters:
 *     G        Default: 1
 *     RO       Default: 1  (would be typically 50 for an op-amp
 *
 *   IP ---+           +--+---- OP
 *         |           |  |
 *         RI          I  RO
 *         RI => G =>  I  RO              V(OP) - V(ON) = (V(IP)-V(IN)) * G
 *         RI          I  RO
 *         |           |  |
 *   IN ---+           +--+---- ON
 *
 *   G=1 ==> 1V ==> 1V
 *
 *   RI = 1 / NETLIST_GMIN
 *
 *   Internal GI = G / RO
 *
 */

#define NETDEV_VCVS(_name)                                                         \
		NET_REGISTER_DEV(VCVS, _name)
//NETDEV_PARAMI(_name, model, _model)


class NETLIB_NAME(VCVS) : public NETLIB_NAME(VCCS)
{
public:
	ATTR_COLD NETLIB_NAME(VCVS)()
	: NETLIB_NAME(VCCS)(VCVS) { }

protected:
	ATTR_COLD virtual void start();
	ATTR_COLD virtual void update_param();
	//ATTR_HOT ATTR_ALIGN void update();

	netlist_terminal_t m_OP2;
	netlist_terminal_t m_ON2;

	netlist_param_double_t m_RO;
};


#endif /* NLD_TWOTERM_H_ */
