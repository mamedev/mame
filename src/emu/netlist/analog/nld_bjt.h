// license:???
// copyright-holders:???
/*
 * nld_bjt.h
 *
 */

#ifndef NLD_BJT_H_
#define NLD_BJT_H_

#include "../nl_base.h"
#include "nld_twoterm.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define QBJT_SW(_name, _model)                                           \
		NET_REGISTER_DEV(QBJT_switch, _name)                             \
		NETDEV_PARAMI(_name,  model,   _model)

#define QBJT_EB(_name, _model)                                           \
		NET_REGISTER_DEV(QBJT_EB, _name)                                 \
		NETDEV_PARAMI(_name,  model,   _model)


// ----------------------------------------------------------------------------------------
// nld_Q - Base classes
// ----------------------------------------------------------------------------------------

// Have a common start for transistors

class NETLIB_NAME(Q) : public netlist_device_t
{
public:
	enum q_type {
		BJT_NPN,
		BJT_PNP
	};

	ATTR_COLD NETLIB_NAME(Q)(const family_t afamily)
	: netlist_device_t(afamily)
	, m_qtype(BJT_NPN) { }

	inline q_type qtype() const { return m_qtype; }
	inline bool is_qtype(q_type atype) const { return m_qtype == atype; }
	inline void set_qtype(q_type atype) { m_qtype = atype; }
protected:
	ATTR_COLD virtual void start();
	ATTR_COLD virtual void reset();
	ATTR_HOT ATTR_ALIGN void update();

	netlist_param_model_t m_model;
private:
	q_type m_qtype;
};

class NETLIB_NAME(QBJT) : public NETLIB_NAME(Q)
{
public:

	ATTR_COLD NETLIB_NAME(QBJT)(const family_t afamily)
	: NETLIB_NAME(Q)(afamily) { }

protected:

private:
};




// ----------------------------------------------------------------------------------------
// nld_QBJT_switch
// ----------------------------------------------------------------------------------------


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

class NETLIB_NAME(QBJT_switch) : public NETLIB_NAME(QBJT)
{
public:
	ATTR_COLD NETLIB_NAME(QBJT_switch)()
	: NETLIB_NAME(QBJT)(BJT_SWITCH),
		m_RB(netlist_object_t::ANALOG),
		m_RC(netlist_object_t::ANALOG),
		m_BC_dummy(netlist_object_t::ANALOG),
		m_gB(NETLIST_GMIN_DEFAULT), m_gC(NETLIST_GMIN_DEFAULT), m_V(0.0), m_state_on(0) { }

	NETLIB_UPDATE_TERMINALS()
	{
		const nl_double m = (is_qtype( BJT_NPN) ? 1 : -1);

		const int new_state = (m_RB.deltaV() * m > m_V ) ? 1 : 0;
		if (m_state_on ^ new_state)
		{
#if 0
			nl_double gb = m_gB;
			nl_double gc = m_gC;
			nl_double v  = m_V * m;
			if (!new_state )
			{
				// not conducting
				gb = netlist().gmin();
				v = 0;
				gc = netlist().gmin();
			}
#else
			const nl_double gb = new_state ? m_gB : netlist().gmin();
			const nl_double gc = new_state ? m_gC : netlist().gmin();
			const nl_double v  = new_state ? m_V * m : 0;
#endif
			m_RB.set(gb, v,   0.0);
			m_RC.set(gc, 0.0, 0.0);
			//m_RB.update_dev();
			//m_RC.update_dev();
			m_state_on = new_state;
		}
	}

	ATTR_HOT ATTR_ALIGN void virtual update();

	nld_twoterm m_RB;
	nld_twoterm m_RC;

	// FIXME: the matrix solvers should be devices so we can properly
	//        schedule them. This is a workaround and blows netgroup size

	nld_twoterm m_BC_dummy;

protected:

	ATTR_COLD virtual void start();
	ATTR_HOT void update_param();

	nl_double m_gB; // base conductance / switch on
	nl_double m_gC; // collector conductance / switch on
	nl_double m_V; // internal voltage source
	UINT8 m_state_on;

private:
};

// ----------------------------------------------------------------------------------------
// nld_QBJT_EB
// ----------------------------------------------------------------------------------------


class NETLIB_NAME(QBJT_EB) : public NETLIB_NAME(QBJT)
{
public:
	ATTR_COLD NETLIB_NAME(QBJT_EB)()
	: NETLIB_NAME(QBJT)(BJT_EB),
		m_D_CB(netlist_object_t::ANALOG),
		m_D_EB(netlist_object_t::ANALOG),
		m_D_EC(netlist_object_t::ANALOG),
		m_alpha_f(0),
		m_alpha_r(0)
		{ }

	NETLIB_UPDATE_TERMINALS()
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

		m_D_EB.set_mat(gee, gec - gee, gce - gee, gee - gec, Ie, -Ie);
		m_D_CB.set_mat(gcc, gce - gcc, gec - gcc, gcc - gce, Ic, -Ic);
		m_D_EC.set_mat( 0,    -gec,      -gce,        0,       0,   0);
	}

protected:

	ATTR_COLD virtual void start();
	ATTR_COLD virtual void reset();
	ATTR_HOT void update_param();
	ATTR_HOT ATTR_ALIGN void virtual update();

	netlist_generic_diode m_gD_BC;
	netlist_generic_diode m_gD_BE;

	nld_twoterm m_D_CB;  // gcc, gce - gcc, gec - gcc, gcc - gce | Ic
	nld_twoterm m_D_EB;  // gee, gec - gee, gce - gee, gee - gec | Ie
	nld_twoterm m_D_EC;  // 0, -gec, -gcc, 0 | 0

	nl_double m_alpha_f;
	nl_double m_alpha_r;

private:
};

#endif /* NLD_BJT_H_ */
