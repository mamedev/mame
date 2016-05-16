// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_R2R_dac.c
 *
 */

#include "nld_r2r_dac.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_UPDATE_PARAM(r2r_dac)
{
	update_dev();

	nl_double V = m_VIN.Value() / (nl_double) (1 << m_num.Value()) * (nl_double) m_val.Value();

	this->set(1.0 / m_R.Value(), V, 0.0);
}

NETLIB_NAMESPACE_DEVICES_END()
