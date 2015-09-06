// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_R2R_dac.c
 *
 */

#include "nld_r2r_dac.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_START(r2r_dac)
{
	NETLIB_NAME(twoterm)::start();
	register_terminal("VOUT", m_P);
	register_terminal("VGND", m_N);
	register_param("R", m_R, 1.0);
	register_param("VIN", m_VIN, 1.0);
	register_param("N", m_num, 1);
	register_param("VAL", m_val, 1);
}

NETLIB_RESET(r2r_dac)
{
	NETLIB_NAME(twoterm)::reset();
}

NETLIB_UPDATE(r2r_dac)
{
	NETLIB_NAME(twoterm)::update();
}

NETLIB_UPDATE_PARAM(r2r_dac)
{
	update_dev();

	nl_double V = m_VIN.Value() / (nl_double) (1 << m_num.Value()) * (nl_double) m_val.Value();

	this->set(1.0 / m_R.Value(), V, 0.0);
}

NETLIB_NAMESPACE_DEVICES_END()
