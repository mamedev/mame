// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_bjt.h
 *
 */

#ifndef NLD_BJT_H_
#define NLD_BJT_H_

#include "nl_base.h"
#include "nld_twoterm.h"

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

#define QBJT_SW(name, model)                                                 \
		NET_REGISTER_DEV(QBJT_SW, name)                                       \
		NETDEV_PARAMI(name,  MODEL, model)

#define QBJT_EB(name, model)                                                 \
		NET_REGISTER_DEV(QBJT_EB, name)                                       \
		NETDEV_PARAMI(name,  MODEL, model)


NETLIB_NAMESPACE_DEVICES_START()

// -----------------------------------------------------------------------------
// nld_Q - Base classes
// -----------------------------------------------------------------------------

// Have a common start for transistors

NETLIB_OBJECT(Q)
{
public:
	enum q_type {
		BJT_NPN,
		BJT_PNP
	};

	NETLIB_CONSTRUCTOR(Q)
	, m_model(*this, "MODEL", "")
	, m_qtype(BJT_NPN)
	{
	}

	NETLIB_DYNAMIC()

	//NETLIB_RESETI();
	NETLIB_UPDATEI();

	inline q_type qtype() const { return m_qtype; }
	inline bool is_qtype(q_type atype) const { return m_qtype == atype; }
	inline void set_qtype(q_type atype) { m_qtype = atype; }
protected:

	param_model_t m_model;
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
		, m_RB(owner, "m_RB")
		, m_RC(owner, "m_RC")
		, m_BC_dummy(owner, "m_BC")
		, m_gB(NETLIST_GMIN_DEFAULT)
		, m_gC(NETLIST_GMIN_DEFAULT)
		, m_V(0.0)
		, m_state_on(0)
	{
		enregister("B", m_RB.m_P);
		enregister("E", m_RB.m_N);
		enregister("C", m_RC.m_P);
		enregister("_E1", m_RC.m_N);

		enregister("_B1", m_BC_dummy.m_P);
		enregister("_C1", m_BC_dummy.m_N);

		connect_late(m_RB.m_N, m_RC.m_N);

		connect_late(m_RB.m_P, m_BC_dummy.m_P);
		connect_late(m_RC.m_P, m_BC_dummy.m_N);

		save(NLNAME(m_state_on));
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
	UINT8 m_state_on;

private:
};

// -----------------------------------------------------------------------------
// nld_QBJT_EB
// -----------------------------------------------------------------------------


NETLIB_OBJECT_DERIVED(QBJT_EB, QBJT)
{
public:
	NETLIB_CONSTRUCTOR_DERIVED(QBJT_EB, QBJT)
		,   m_D_CB(owner, "m_D_CB")
		,   m_D_EB(owner, "m_D_EB")
		,   m_D_EC(owner, "m_D_EC")
		,   m_alpha_f(0)
		,   m_alpha_r(0)
	{
		enregister("E", m_D_EB.m_P);   // Cathode
		enregister("B", m_D_EB.m_N);   // Anode

		enregister("C", m_D_CB.m_P);   // Cathode
		enregister("_B1", m_D_CB.m_N); // Anode

		enregister("_E1", m_D_EC.m_P);
		enregister("_C1", m_D_EC.m_N);

		connect_late(m_D_EB.m_P, m_D_EC.m_P);
		connect_late(m_D_EB.m_N, m_D_CB.m_N);
		connect_late(m_D_CB.m_P, m_D_EC.m_N);

		m_gD_BE.save("m_D_BE", *this);
		m_gD_BC.save("m_D_BC", *this);
	}

protected:

	NETLIB_RESETI();
	NETLIB_UPDATEI();
	NETLIB_UPDATE_PARAMI();
	NETLIB_UPDATE_TERMINALSI();

	generic_diode m_gD_BC;
	generic_diode m_gD_BE;

	nld_twoterm m_D_CB;  // gcc, gce - gcc, gec - gcc, gcc - gce | Ic
	nld_twoterm m_D_EB;  // gee, gec - gee, gce - gee, gee - gec | Ie
	nld_twoterm m_D_EC;  // 0, -gec, -gcc, 0 | 0

	nl_double m_alpha_f;
	nl_double m_alpha_r;

private:
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_BJT_H_ */
