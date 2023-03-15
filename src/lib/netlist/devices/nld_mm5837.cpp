// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_MM5837.cpp
 *
 *  MM5837: Digital noise source
 *
 *          +--------+
 *      VDD |1  ++  8| NC
 *      VGG |2      7| NC
 *      OUT |3      6| NC
 *      VSS |4      5| NC
 *          +--------+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#include "analog/nlid_twoterm.h"
#include "solver/nld_matrix_solver.h"

namespace netlist::devices {

	NETLIB_OBJECT(MM5837)
	{
		NETLIB_CONSTRUCTOR(MM5837)
		, m_RV(*this, "_RV")
		, m_VDD(*this, "VDD", NETLIB_DELEGATE(inputs))
		, m_VGG(*this, "VGG", NETLIB_DELEGATE(inputs))
		, m_VSS(*this, "VSS", NETLIB_DELEGATE(inputs))
		, m_FREQ(*this, "FREQ", 24000 * 2)
		, m_R_LOW(*this, "R_LOW", 1000)
		, m_R_HIGH(*this, "R_HIGH", 1000)
		/* clock */
		, m_feedback(*this, "_FB", NETLIB_DELEGATE(inputs))
		, m_Q(*this, "_Q")
		, m_inc(netlist_time::from_hz(24000 * 2))
		, m_shift(*this, "m_shift", 0)
		{
			connect("_FB", "_Q");

			// output
			connect("_RV.2", "VDD");
			register_sub_alias("OUT", "_RV.1");
		}

		NETLIB_RESETI()
		{
			//m_V0.initial(0.0);
			//m_RV.do_reset();
			m_RV().set_G_V_I(plib::reciprocal(m_R_LOW()),
				nlconst::zero(),
				nlconst::zero());
			m_inc = netlist_time::from_fp(plib::reciprocal(m_FREQ()));
			if (m_FREQ() < nlconst::magic(24000*2) || m_FREQ() > nlconst::magic(56000*2))
				log().warning(MW_FREQUENCY_OUTSIDE_OF_SPECS_1(m_FREQ()));

			m_shift = 0x1ffff;
		}

		NETLIB_UPDATE_PARAMI()
		{
			m_inc = netlist_time::from_fp(plib::reciprocal(m_FREQ()));
			if (m_FREQ() < nlconst::magic(24000*2) || m_FREQ() > nlconst::magic(56000*2))
				log().warning(MW_FREQUENCY_OUTSIDE_OF_SPECS_1(m_FREQ()));
		}

	private:
		NETLIB_HANDLERI(inputs)
		{
			m_Q.push(!m_feedback(), m_inc);

			/* shift register
			 *
			 * 17 bits, bits 17 & 14 feed back to input
			 *
			 */

			const auto last_state = m_shift & 0x01;
			/* shift */
			m_shift = (m_shift >> 1) | (((m_shift & 0x01) ^ ((m_shift >> 3) & 0x01)) << 16);
			const auto state = m_shift & 0x01;

			if (state != last_state)
			{
				const nl_fptype R = state ? m_R_HIGH : m_R_LOW;
				const nl_fptype V = state ? m_VDD() : m_VSS();

				m_RV().change_state([this, &R, &V]()
				{
					m_RV().set_G_V_I(plib::reciprocal(R), V, nlconst::zero());
				});
			}

		}

		NETLIB_SUB_NS(analog, two_terminal) m_RV;
		analog_input_t m_VDD;
		analog_input_t m_VGG;
		analog_input_t m_VSS;
		param_fp_t m_FREQ;
		param_fp_t m_R_LOW;
		param_fp_t m_R_HIGH;

		/* clock stage */
		logic_input_t m_feedback;
		logic_output_t m_Q;
		netlist_time m_inc;

		/* state */
		state_var_u32 m_shift;
	};

	NETLIB_DEVICE_IMPL(MM5837, "MM5837", "")

} // namespace netlist::devices
