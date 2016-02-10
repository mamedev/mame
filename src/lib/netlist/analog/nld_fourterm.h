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

#define VCCS(_name)                                                            \
		NET_REGISTER_DEV(VCCS, _name)

#define CCCS(_name)                                                            \
		NET_REGISTER_DEV(CCCS, _name)

#define VCVS(_name)                                                            \
		NET_REGISTER_DEV(VCVS, _name)

#define LVCCS(_name)                                                           \
		NET_REGISTER_DEV(LVCCS, _name)

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

class NETLIB_NAME(VCCS) : public device_t
{
public:
	ATTR_COLD NETLIB_NAME(VCCS)()
	: device_t(VCCS), m_gfac(1.0) {  }
	ATTR_COLD NETLIB_NAME(VCCS)(const family_t afamily)
	: device_t(afamily), m_gfac(1.0) {  }

	param_double_t m_G;
	param_double_t m_RI;

protected:
	virtual void start() override;
	virtual void reset() override;
	virtual void update_param() override;
	ATTR_HOT virtual void update() override;

	ATTR_COLD void start_internal(const nl_double def_RI);

	terminal_t m_OP;
	terminal_t m_ON;

	terminal_t m_IP;
	terminal_t m_IN;

	terminal_t m_OP1;
	terminal_t m_ON1;

	nl_double m_gfac;
};

/* Limited Current source*/

class NETLIB_NAME(LVCCS) : public NETLIB_NAME(VCCS)
{
public:
	ATTR_COLD NETLIB_NAME(LVCCS)()
	: NETLIB_NAME(VCCS)(LVCCS), m_vi(0.0) {  }
	ATTR_COLD NETLIB_NAME(LVCCS)(const family_t afamily)
	: NETLIB_NAME(VCCS)(afamily), m_vi(0.0) {  }

	param_double_t m_cur_limit; /* current limit */

protected:
	virtual void start() override;
	virtual void reset() override;
	virtual void update_param() override;
	ATTR_HOT virtual void update() override;
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

class NETLIB_NAME(CCCS) : public NETLIB_NAME(VCCS)
{
public:
	ATTR_COLD NETLIB_NAME(CCCS)()
	: NETLIB_NAME(VCCS)(CCCS), m_gfac(1.0) {  }

protected:
	virtual void start() override;
	virtual void reset() override;
	virtual void update_param() override;
	ATTR_HOT void update() override;

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


class NETLIB_NAME(VCVS) : public NETLIB_NAME(VCCS)
{
public:
	ATTR_COLD NETLIB_NAME(VCVS)()
	: NETLIB_NAME(VCCS)(VCVS) { }

	param_double_t m_RO;

protected:
	virtual void start() override;
	virtual void reset() override;
	virtual void update_param() override;
	//ATTR_HOT void update();

	terminal_t m_OP2;
	terminal_t m_ON2;

};

NETLIB_NAMESPACE_DEVICES_END()


#endif /* NLD_FOURTERM_H_ */
