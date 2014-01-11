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
		NETDEV_PARAMI(_name, model, _model)

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

    ATTR_HOT inline double deltaV() { return m_P.net().Q_Analog()- m_N.net().Q_Analog(); }

protected:
	ATTR_COLD virtual void start();
    ATTR_COLD virtual void reset();
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
    ATTR_COLD virtual void reset();
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
		double I = -G * deltaV();
		set(G, 0.0, I);
	}

protected:
	ATTR_COLD virtual void start();
	ATTR_COLD virtual void update_param();
	ATTR_HOT ATTR_ALIGN void update();

	netlist_param_double_t m_C;

};


// ----------------------------------------------------------------------------------------
// A generic diode model to be used in other devices (Diode, BJT ...)
// ----------------------------------------------------------------------------------------

class netlist_generic_diode
{
public:
    netlist_generic_diode()
    {
        m_Vd = 0.7;
    }

    ATTR_HOT inline void update_diode(const double nVd)
    {
        //FIXME: Optimize cutoff case

        if (nVd < -5.0 * m_Vt)
        {
            m_Vd = nVd;
            m_G = NETLIST_GMIN;
            m_Id = - m_Is;
        }
        else if (nVd < m_Vcrit)
        {
            m_Vd = nVd;

            const double eVDVt = exp(m_Vd * m_VtInv);
            m_Id = m_Is * (eVDVt - 1.0);
            m_G = m_Is * m_VtInv * eVDVt;
        }
        else
        {
#if defined(_MSC_VER) && _MSC_VER < 1800
            m_Vd = m_Vd + log((nVd - m_Vd) * m_VtInv + 1.0) * m_Vt;
#else
            m_Vd = m_Vd + log1p((nVd - m_Vd) * m_VtInv) * m_Vt;
#endif
            const double eVDVt = exp(m_Vd * m_VtInv);
            m_Id = m_Is * (eVDVt - 1.0);

            m_G = m_Is * m_VtInv * eVDVt;
        }

        //printf("nVd %f m_Vd %f Vcrit %f\n", nVd, m_Vd, m_Vcrit);
    }

    void set_param(const double Is, const double n)
    {
        m_Is = Is;
        m_n = n;

        m_Vt = 0.0258 * m_n;

        m_Vcrit = m_Vt * log(m_Vt / m_Is / sqrt(2.0));
        m_VtInv = 1.0 / m_Vt;
    }

    ATTR_HOT inline double I() { return m_Id; }
    ATTR_HOT inline double G() { return m_G; }
    ATTR_HOT inline double Ieq() { return (m_Id - m_Vd * m_G); }

    /* owning object must save those ... */

    void save(pstring name, netlist_object_t &parent)
    {
        parent.save(m_Vd, name + ".m_Vd");
        parent.save(m_Id, name + ".m_Id");
        parent.save(m_G, name + ".m_G");
    }

private:
    double m_Vd;
    double m_Id;
    double m_G;

    double m_Vt;
    double m_Is;
    double m_n;

    double m_VtInv;
    double m_Vcrit;
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
	    m_D.update_diode(deltaV());
		set(m_D.G(), 0.0, m_D.Ieq());
	}

protected:
	ATTR_COLD virtual void start();
	ATTR_COLD virtual void update_param();
	ATTR_HOT ATTR_ALIGN void update();

	netlist_param_model_t m_model;

	netlist_generic_diode m_D;
};



#endif /* NLD_TWOTERM_H_ */
