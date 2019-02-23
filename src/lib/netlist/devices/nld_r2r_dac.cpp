// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_R2R_dac.c
 *
 */

#include "../nl_base.h"
#include "../analog/nlid_twoterm.h"
#include "../nl_factory.h"

namespace netlist
{
	namespace analog
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
		solve_now();

		double V = m_VIN() / static_cast<double>(1 << m_num())
				* static_cast<double>(m_val());

		this->set(1.0 / m_R(), V, 0.0);
	}
	} //namespace analog

	namespace devices {
		NETLIB_DEVICE_IMPL_NS(analog, r2r_dac, "R2R_DAC", "VIN,R,N")
	} // namespace devices

} // namespace netlist
