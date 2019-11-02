// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_R2R_dac.c
 *
 */

#include "netlist/nl_base.h"
#include "netlist/analog/nlid_twoterm.h"
#include "netlist/nl_factory.h"

namespace netlist
{
	namespace analog
	{
	NETLIB_OBJECT_DERIVED(r2r_dac, twoterm)
	{
		NETLIB_CONSTRUCTOR_DERIVED(r2r_dac, twoterm)
		, m_VIN(*this, "VIN", nlconst::one())
		, m_R(*this, "R", nlconst::one())
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
		param_fp_t m_VIN;
		param_fp_t m_R;
		param_int_t m_num;
		param_int_t m_val;
	};


	NETLIB_UPDATE_PARAM(r2r_dac)
	{
		solve_now();

		nl_fptype V = m_VIN() / static_cast<nl_fptype>(1 << m_num())
				* static_cast<nl_fptype>(m_val());

		this->set_G_V_I(plib::reciprocal(m_R()), V, nlconst::zero());
	}
	} //namespace analog

	namespace devices {
		NETLIB_DEVICE_IMPL_NS(analog, r2r_dac, "R2R_DAC", "VIN,R,N")
	} // namespace devices

} // namespace netlist
