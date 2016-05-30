// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_fourterm.h
 *
 */

#ifndef NLD_FOURTERM_H_
#define NLD_FOURTERM_H_


#include "nl_base.h"
#include "nld_twoterm.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define VCCS(name)                                                            \
		NET_REGISTER_DEV(VCCS, name)

#define CCCS(name)                                                            \
		NET_REGISTER_DEV(CCCS, name)

#define VCVS(name)                                                            \
		NET_REGISTER_DEV(VCVS, name)

#define LVCCS(name)                                                           \
		NET_REGISTER_DEV(LVCCS, name)

NETLIB_NAMESPACE_DEVICES_START()

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

NETLIB_OBJECT(VCCS)
{
public:
	NETLIB_CONSTRUCTOR(VCCS)
	, m_G(*this, "G", 1.0)
	, m_RI(*this, "RI", NL_FCONST(1.0) / netlist().gmin())
	, m_gfac(1.0)
	{

		enregister("IP", m_IP);
		enregister("IN", m_IN);
		enregister("OP", m_OP);
		enregister("ON", m_ON);

		enregister("_OP1", m_OP1);
		enregister("_ON1", m_ON1);

		m_IP.m_otherterm = &m_IN; // <= this should be NULL and terminal be filtered out prior to solving...
		m_IN.m_otherterm = &m_IP; // <= this should be NULL and terminal be filtered out prior to solving...

		m_OP.m_otherterm = &m_IP;
		m_OP1.m_otherterm = &m_IN;

		m_ON.m_otherterm = &m_IP;
		m_ON1.m_otherterm = &m_IN;

		connect_late(m_OP, m_OP1);
		connect_late(m_ON, m_ON1);
		m_gfac = NL_FCONST(1.0);
	}

	param_double_t m_G;
	param_double_t m_RI;

protected:
	NETLIB_RESETI();
	NETLIB_UPDATEI();
	NETLIB_UPDATE_PARAMI()
	{
		NETLIB_NAME(VCCS)::reset();
	}

	terminal_t m_OP;
	terminal_t m_ON;

	terminal_t m_IP;
	terminal_t m_IN;

	terminal_t m_OP1;
	terminal_t m_ON1;

	nl_double m_gfac;
};

/* Limited Current source*/

NETLIB_OBJECT_DERIVED(LVCCS, VCCS)
{
public:
	NETLIB_CONSTRUCTOR_DERIVED(LVCCS, VCCS)
	, m_cur_limit(*this, "CURLIM", 1000.0)
	, m_vi(0.0)
	{
	}

	NETLIB_DYNAMIC()

	param_double_t m_cur_limit; /* current limit */

protected:
	NETLIB_UPDATEI();
	NETLIB_RESETI();
	NETLIB_UPDATE_PARAMI();
	NETLIB_UPDATE_TERMINALSI();

	nl_double m_vi;
};

// ----------------------------------------------------------------------------------------
// nld_CCCS
// ----------------------------------------------------------------------------------------

/*
 *   Current controlled current source
 *
 *   IP ---+           +------> OP
 *         |           |
 *         RI          I
 *         RI => G =>  I    IOut = (V(IP)-V(IN)) / RI  * G
 *         RI          I
 *         |           |
 *   IN ---+           +------< ON
 *
 *   G=1 ==> 1A ==> 1A
 *
 *   RI = 1
 *
 *   This needs high levels of accuracy to work with 1 Ohm RI.
 *
 */

NETLIB_OBJECT_DERIVED(CCCS, VCCS)
{
public:
	NETLIB_CONSTRUCTOR_DERIVED(CCCS, VCCS)
	, m_gfac(1.0)
	{
		m_gfac = NL_FCONST(1.0) / m_RI.Value();
	}

protected:
	NETLIB_UPDATEI();
	NETLIB_RESETI();
	NETLIB_UPDATE_PARAMI();

	nl_double m_gfac;
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


NETLIB_OBJECT_DERIVED(VCVS, VCCS)
{
public:
	NETLIB_CONSTRUCTOR_DERIVED(VCVS, VCCS)
	, m_RO(*this, "RO",1.0)
	{

		enregister("_OP2", m_OP2);
		enregister("_ON2", m_ON2);

		m_OP2.m_otherterm = &m_ON2;
		m_ON2.m_otherterm = &m_OP2;

		connect_late(m_OP2, m_OP1);
		connect_late(m_ON2, m_ON1);
	}

	param_double_t m_RO;

protected:
	//NETLIB_UPDATEI();
	NETLIB_RESETI();
	//NETLIB_UPDATE_PARAMI();

	terminal_t m_OP2;
	terminal_t m_ON2;

};

NETLIB_NAMESPACE_DEVICES_END()


#endif /* NLD_FOURTERM_H_ */
