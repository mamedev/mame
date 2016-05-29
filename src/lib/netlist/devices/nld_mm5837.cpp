// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_MM5837.c
 *
 */

#include "nld_mm5837.h"
#include <solver/nld_matrix_solver.h>
#include "analog/nld_twoterm.h"

#define R_LOW (1000.0)
#define R_HIGH (1000.0)

namespace netlist
{
	namespace devices
	{

	NETLIB_OBJECT(MM5837_dip)
	{
		NETLIB_CONSTRUCTOR(MM5837_dip)
		, m_RV(*this, "RV")
		, m_is_timestep(false)
		{
			/* clock */
			enregister("Q", m_Q);
			enregister("FB", m_feedback);
			m_inc = netlist_time::from_hz(56000);
			connect_late(m_feedback, m_Q);

			/* output */
			enregister("_RV1", m_RV.m_P);
			enregister("_RV2", m_RV.m_N);
			enregister("_Q", m_V0);
			connect_late(m_RV.m_N, m_V0);

			/* device */
			enregister("1", m_VDD);
			enregister("2", m_VGG);
			register_subalias("3", m_RV.m_P);
			enregister("4", m_VSS);

			save(NLNAME(m_shift));

		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		NETLIB_SUB(twoterm) m_RV;
		analog_input_t m_VDD;
		analog_input_t m_VGG;
		analog_input_t m_VSS;

		/* output stage */
		analog_output_t m_V0; /* could be gnd as well */

		/* clock stage */
		logic_input_t m_feedback;
		logic_output_t m_Q;
		netlist_time m_inc;

		/* state */
		UINT32 m_shift;

		/* cache */
		bool m_is_timestep;
	};

	NETLIB_RESET(MM5837_dip)
	{
		//m_V0.initial(0.0);
		//m_RV.do_reset();
		m_RV.set(NL_FCONST(1.0) / R_LOW, 0.0, 0.0);

		m_shift = 0x1ffff;
		m_is_timestep = m_RV.m_P.net().solver()->is_timestep();
	}

	NETLIB_UPDATE(MM5837_dip)
	{
		OUTLOGIC(m_Q, !m_Q.net().as_logic().new_Q(), m_inc  );

		/* shift register
		 *
		 * 17 bits, bits 17 & 14 feed back to input
		 *
		 */

		const UINT32 last_state = m_shift & 0x01;
		/* shift */
		m_shift = (m_shift >> 1) | (((m_shift & 0x01) ^ ((m_shift >> 3) & 0x01)) << 16);
		const UINT32 state = m_shift & 0x01;

		if (state != last_state)
		{
			const nl_double R = state ? R_HIGH : R_LOW;
			const nl_double V = state ? INPANALOG(m_VDD) : INPANALOG(m_VSS);

			// We only need to update the net first if this is a time stepping net
			if (m_is_timestep)
				m_RV.update_dev();
			m_RV.set(NL_FCONST(1.0) / R, V, 0.0);
			m_RV.m_P.schedule_after(NLTIME_FROM_NS(1));
		}

	}

	NETLIB_DEVICE_IMPL(MM5837_dip)

	} //namespace devices
} // namespace netlist
