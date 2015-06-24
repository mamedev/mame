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

#define RES(_name, _R)                                                         \
		NET_REGISTER_DEV(R, _name)                                                  \
		NETDEV_PARAMI(_name, R, _R)

#define POT(_name, _R)                                                       \
		NET_REGISTER_DEV(POT, _name)                                                \
		NETDEV_PARAMI(_name, R, _R)

/* Does not have pin 3 connected */
#define POT2(_name, _R)                                                       \
		NET_REGISTER_DEV(POT2, _name)                                                \
		NETDEV_PARAMI(_name, R, _R)


#define CAP(_name, _C)                                                         \
		NET_REGISTER_DEV(C, _name)                                                  \
		NETDEV_PARAMI(_name, C, _C)

/* Generic Diode */
#define DIODE(_name,  _model)                                                    \
		NET_REGISTER_DEV(D, _name)                                                  \
		NETDEV_PARAMI(_name, model, _model)

// ----------------------------------------------------------------------------------------
// Generic macros
// ----------------------------------------------------------------------------------------


#ifdef RES_R
// FIXME: avoid compile fails
// #warning "Do not include rescap.h in a netlist environment"
#endif

#define RES_R(res) ((double)(res))
#define RES_K(res) ((double)(res) * 1e3)
#define RES_M(res) ((double)(res) * 1e6)
#define CAP_U(cap) ((double)(cap) * 1e-6)
#define CAP_N(cap) ((double)(cap) * 1e-9)
#define CAP_P(cap) ((double)(cap) * 1e-12)
#define IND_U(ind) ((double)(ind) * 1e-6)
#define IND_N(ind) ((double)(ind) * 1e-9)
#define IND_P(ind) ((double)(ind) * 1e-12)

// ----------------------------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------------------------

NETLIB_NAMESPACE_DEVICES_START()

// ----------------------------------------------------------------------------------------
// nld_twoterm
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(twoterm) : public device_t
{
public:
	ATTR_COLD NETLIB_NAME(twoterm)(const family_t afamily);
	ATTR_COLD NETLIB_NAME(twoterm)();

	terminal_t m_P;
	terminal_t m_N;

	virtual NETLIB_UPDATE_TERMINALSI()
	{
	}

	ATTR_HOT /* inline */ void set(const nl_double G, const nl_double V, const nl_double I)
	{
		/*      GO, GT, I                */
		m_P.set( G,  G, (  V) * G - I);
		m_N.set( G,  G, ( -V) * G + I);
	}

	ATTR_HOT /* inline */ nl_double deltaV() const
	{
		return m_P.net().as_analog().Q_Analog() - m_N.net().as_analog().Q_Analog();
	}

	ATTR_HOT void set_mat(nl_double a11, nl_double a12, nl_double a21, nl_double a22, nl_double r1, nl_double r2)
	{
		/*      GO, GT, I                */
		m_P.set(-a12, a11, -r1);
		m_N.set(-a21, a22, -r2);
	}

protected:
	virtual void start();
	virtual void reset();
	ATTR_HOT void update();

private:
};

// ----------------------------------------------------------------------------------------
// nld_R
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(R_base) : public NETLIB_NAME(twoterm)
{
public:
	ATTR_COLD NETLIB_NAME(R_base)() : NETLIB_NAME(twoterm)(RESISTOR) { }

	inline void set_R(const nl_double R)
	{
		set(NL_FCONST(1.0) / R, 0.0, 0.0);
	}

protected:
	virtual void start();
	virtual void reset();
	ATTR_HOT void update();
};

NETLIB_DEVICE_WITH_PARAMS_DERIVED(R, R_base,
	param_double_t m_R;
);

// ----------------------------------------------------------------------------------------
// nld_POT
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(POT,
	NETLIB_NAME(R_base) m_R1;
	NETLIB_NAME(R_base) m_R2;

	param_double_t m_R;
	param_double_t m_Dial;
	param_logic_t m_DialIsLog;
);

NETLIB_DEVICE_WITH_PARAMS(POT2,
	NETLIB_NAME(R_base) m_R1;

	param_double_t m_R;
	param_double_t m_Dial;
	param_logic_t m_DialIsLog;
	param_logic_t m_Reverse;
);


// ----------------------------------------------------------------------------------------
// nld_C
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(C) : public NETLIB_NAME(twoterm)
{
public:
	ATTR_COLD NETLIB_NAME(C)() : NETLIB_NAME(twoterm)(CAPACITOR) { }

	ATTR_HOT void step_time(const nl_double st)
	{
		const nl_double G = m_C.Value() / st;
		const nl_double I = -G * deltaV();
		set(G, 0.0, I);
	}

protected:
	virtual void start();
	virtual void reset();
	virtual void update_param();
	ATTR_HOT void update();

	param_double_t m_C;

};


// ----------------------------------------------------------------------------------------
// A generic diode model to be used in other devices (Diode, BJT ...)
// ----------------------------------------------------------------------------------------

class generic_diode
{
public:
	ATTR_COLD generic_diode();

	ATTR_HOT inline void update_diode(const nl_double nVd)
	{
		//FIXME: Optimize cutoff case

		if (nVd < NL_FCONST(-5.0) * m_Vt)
		{
			m_Vd = nVd;
			m_G = m_gmin;
			m_Id = - m_Is;
		}
		else if (nVd < m_Vcrit)
		{
			m_Vd = nVd;

			const nl_double eVDVt = nl_math::exp(m_Vd * m_VtInv);
			m_Id = m_Is * (eVDVt - NL_FCONST(1.0));
			m_G = m_Is * m_VtInv * eVDVt + m_gmin;
		}
		else
		{
			nl_double a = (nVd - m_Vd) * m_VtInv;
			if (a < NL_FCONST(1e-12) - NL_FCONST(1.0)) a = NL_FCONST(1e-12) - NL_FCONST(1.0);
			m_Vd = m_Vd + nl_math::e_log1p(a) * m_Vt;

			const nl_double eVDVt = nl_math::exp(m_Vd * m_VtInv);
			m_Id = m_Is * (eVDVt - NL_FCONST(1.0));

			m_G = m_Is * m_VtInv * eVDVt + m_gmin;
		}

		//printf("nVd %f m_Vd %f Vcrit %f\n", nVd, m_Vd, m_Vcrit);
	}

	ATTR_COLD void set_param(const nl_double Is, const nl_double n, nl_double gmin);

	ATTR_HOT inline nl_double I() const { return m_Id; }
	ATTR_HOT inline nl_double G() const { return m_G; }
	ATTR_HOT inline nl_double Ieq() const { return (m_Id - m_Vd * m_G); }
	ATTR_HOT inline nl_double Vd() const { return m_Vd; }

	/* owning object must save those ... */

	ATTR_COLD void save(pstring name, object_t &parent);

private:
	nl_double m_Vd;
	nl_double m_Id;
	nl_double m_G;

	nl_double m_Vt;
	nl_double m_Is;
	nl_double m_n;
	nl_double m_gmin;

	nl_double m_VtInv;
	nl_double m_Vcrit;
};

// ----------------------------------------------------------------------------------------
// nld_D
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(D) : public NETLIB_NAME(twoterm)
{
public:
	ATTR_COLD NETLIB_NAME(D)() : NETLIB_NAME(twoterm)(DIODE) { }

	NETLIB_UPDATE_TERMINALSI();

protected:
	virtual void start();
	virtual void update_param();
	ATTR_HOT void update();

	param_model_t m_model;

	generic_diode m_D;
};


NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_TWOTERM_H_ */
