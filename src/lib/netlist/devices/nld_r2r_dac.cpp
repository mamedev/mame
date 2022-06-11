// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_r2r_dac.cpp
 *
 *  DMR2R_DAC: R-2R DAC
 *
 *  Generic R-2R DAC ... This is fast.
 *                 2R
 *  Bit n    >----RRR----+---------> Vout
 *                       |
 *                       R
 *                       R R
 *                       R
 *                       |
 *                       .
 *                       .
 *                 2R    |
 *  Bit 2    >----RRR----+
 *                       |
 *                       R
 *                       R R
 *                       R
 *                       |
 *                 2R    |
 *  Bit 1    >----RRR----+
 *                       |
 *                       R
 *                       R 2R
 *                       R
 *                       |
 *                      V0
 *
 * Using Thevenin's Theorem, this can be written as
 *
 *          +---RRR-----------> Vout
 *          |
 *          V
 *          V  V = VAL / 2^n * Vin
 *          V
 *          |
 *          V0
 *
 */

#include "nl_base.h"
#include "analog/nlid_twoterm.h"
#include "nl_factory.h"

namespace netlist::analog {

	NETLIB_OBJECT_DERIVED(r2r_dac, two_terminal)
	{
		NETLIB_CONSTRUCTOR(r2r_dac)
		, m_VIN(*this, "VIN", nlconst::one())
		, m_R(*this, "R", nlconst::one())
		, m_num(*this, "N", 1)
		, m_val(*this, "VAL", 1)
		{
			register_sub_alias("VOUT", P());
			register_sub_alias("VGND", N());
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
		nl_fptype V = m_VIN() / static_cast<nl_fptype>(1 << m_num())
				* static_cast<nl_fptype>(m_val());
		change_state([this, &V]()
			{
				this->set_G_V_I(plib::reciprocal(m_R()), V, nlconst::zero());
			}
		);
	}

} // namespace netlist::analog

namespace netlist::devices {

	NETLIB_DEVICE_IMPL_NS(analog, r2r_dac, "R2R_DAC", "VIN,R,N")

} // namespace netlist::devices
