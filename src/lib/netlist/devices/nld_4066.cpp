// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_4066.cpp
 *
 *  CD4066: Quad Bilateral Switch
 *
 *          +--------------+
 *   INOUTA |1     ++    14| VDD
 *   OUTINA |2           13| CONTROLA
 *   OUTINB |3           12| CONTROLD
 *   INOUTB |4    4066   11| INOUTD
 * CONTROLB |5           10| OUTIND
 * CONTROLC |6            9| OUTINC
 *      VSS |7            8| INOUTC
 *          +--------------+
 *
 *  FIXME: These devices are slow (~125 ns). This is currently not reflected
 *
 *  Naming conventions follow National semiconductor datasheet
 *
 */

#include "analog/nlid_twoterm.h"
#include "solver/nld_solver.h"


namespace netlist::devices {

	NETLIB_OBJECT(CD4066_GATE)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4066_GATE, "CD4XXX")
		, m_R(*this, "R")
		, m_control(*this, "CTL", NETLIB_DELEGATE(control))
		, m_base_r(*this, "BASER", nlconst::magic(270.0))
		, m_last(*this, "m_last", false)
		, m_supply(*this)
		{
		}

		NETLIB_RESETI()
		{
			// Start in off condition
			// FIXME: is ROFF correct?
			m_R().set_R(plib::reciprocal(exec().gmin()));
		}

	private:
		NETLIB_HANDLERI(control)
		{
			nl_fptype sup = (m_supply.VCC().Q_Analog() - m_supply.GND().Q_Analog());
			nl_fptype in = m_control() - m_supply.GND().Q_Analog();
			nl_fptype rON = m_base_r() * nlconst::magic(5.0) / sup;
			nl_fptype R = -nlconst::one();
			nl_fptype low = nlconst::magic(0.45) * sup;
			nl_fptype high = nlconst::magic(0.55) * sup;
			bool new_state(false);
			if (in < low)
			{
				R = plib::reciprocal(exec().gmin());
			}
			else if (in > high)
			{
				R = rON;
				new_state = true;
			}
			if (R > nlconst::zero() && (m_last != new_state))
			{
				m_last = new_state;
				m_R().change_state([this, &R]() -> void { this->m_R().set_R(R);});
			}
		}

		NETLIB_SUB_NS(analog, R_base) m_R;
		analog_input_t             m_control;
		param_fp_t                 m_base_r;
		state_var<bool>            m_last;
		nld_power_pins             m_supply;
	};

	NETLIB_DEVICE_IMPL(CD4066_GATE,         "CD4066_GATE",            "")

} // namespace netlist::devices
