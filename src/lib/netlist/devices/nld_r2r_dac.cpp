// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_R2R_dac.c
 *
 */

#include "nld_r2r_dac.h"
#include "analog/nld_twoterm.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT_DERIVED(r2r_dac, twoterm)
	{
		NETLIB_CONSTRUCTOR_DERIVED(r2r_dac, twoterm)
		, m_VIN(*this, "VIN", 1.0)
		, m_R(*this, "R", 1.0)
		, m_num(*this, "N", 1)
		, m_val(*this, "VAL", 1)
		{
			register_subalias("VOUT", m_P);
			register_subalias("VGND", m_N);
		}

		NETLIB_UPDATE_PARAMI();
		//NETLIB_RESETI();
		//NETLIB_UPDATEI();

	protected:
		param_double_t m_VIN;
		param_double_t m_R;
		param_int_t m_num;
		param_int_t m_val;
	};


	NETLIB_UPDATE_PARAM(r2r_dac)
	{
		update_dev();

		nl_double V = m_VIN.Value() / (nl_double) (1 << m_num.Value()) * (nl_double) m_val.Value();

		this->set(1.0 / m_R.Value(), V, 0.0);
	}

	NETLIB_DEVICE_IMPL(r2r_dac)

	} //namespace devices
} // namespace netlist
