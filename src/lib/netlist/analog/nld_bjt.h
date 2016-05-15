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

#define QBJT_SW(_name, _model)                                                 \
		NET_REGISTER_DEV(QBJT_SW, _name)                                       \
		NETDEV_PARAMI(_name,  MODEL,   _model)

#define QBJT_EB(_name, _model)                                                 \
		NET_REGISTER_DEV(QBJT_EB, _name)                                       \
		NETDEV_PARAMI(_name,  MODEL,   _model)


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
	, m_qtype(BJT_NPN)
	{ }

	NETLIB_DYNAMIC()

	inline q_type qtype() const { return m_qtype; }
	inline bool is_qtype(q_type atype) const { return m_qtype == atype; }
	inline void set_qtype(q_type atype) { m_qtype = atype; }
protected:
	virtual void start() override;
	virtual void reset() override;
	ATTR_HOT void update() override;

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
public:
	NETLIB_CONSTRUCTOR_DERIVED(QBJT_switch, QBJT)
		, m_RB(owner, "m_RB")
		, m_RC(owner, "m_RC")
		, m_BC_dummy(owner, "m_BC")
		, m_gB(NETLIST_GMIN_DEFAULT)
		, m_gC(NETLIST_GMIN_DEFAULT)
		, m_V(0.0)
		, m_state_on(0) { }

	ATTR_HOT void virtual update() override;

	nld_twoterm m_RB;
	nld_twoterm m_RC;

	// FIXME: this is needed so we have all terminals belong to one net list

	nld_twoterm m_BC_dummy;

protected:

	virtual void start() override;
	ATTR_HOT virtual void update_param() override;
	virtual void reset() override;
	NETLIB_UPDATE_TERMINALSI();

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
	  ,	m_D_CB(owner, "m_D_CB")
	  ,	m_D_EB(owner, "m_D_EB")
	  ,	m_D_EC(owner, "m_D_EC")
	  ,	m_alpha_f(0)
	  ,	m_alpha_r(0)
		{ }

protected:

	virtual void start() override;
	virtual void reset() override;
	ATTR_HOT void update_param() override;
	ATTR_HOT void virtual update() override;
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
