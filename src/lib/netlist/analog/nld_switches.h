// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_switches.h
 *
 */

#pragma once

#ifndef NLD_SWITCHES_H_
#define NLD_SWITCHES_H_

#include "nl_base.h"
#include "nld_twoterm.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define SWITCH(name)                                                              \
		NET_REGISTER_DEV(SWITCH, name)

#define SWITCH2(name)                                                              \
		NET_REGISTER_DEV(SWITCH2, name)

// ----------------------------------------------------------------------------------------
// Devices ...
// ----------------------------------------------------------------------------------------

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(switch1)
{
	NETLIB_CONSTRUCTOR(switch1)
	, m_R(*this, "R")
	, m_POS(*this, "POS", 0)
	{
		register_subalias("1", m_R.m_P);
		register_subalias("2", m_R.m_N);
	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();
	NETLIB_UPDATE_PARAMI();

	NETLIB_SUB(R_base) m_R;
	param_int_t m_POS;
};

NETLIB_OBJECT(switch2)
{
	NETLIB_CONSTRUCTOR(switch2)
	, m_R1(*this, "R1")
	, m_R2(*this, "R2")
	, m_POS(*this, "POS", 0)
	{
		connect_late(m_R1.m_N, m_R2.m_N);

		register_subalias("1", m_R1.m_P);
		register_subalias("2", m_R2.m_P);

		register_subalias("Q", m_R1.m_N);
	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();
	NETLIB_UPDATE_PARAMI();

	NETLIB_SUB(R_base) m_R1;
	NETLIB_SUB(R_base) m_R2;
	param_int_t m_POS;
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_SWITCHES_H_ */
