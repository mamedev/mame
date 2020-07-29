// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4053.c
 *
 */

#include "nld_4053.h"

#include "netlist/analog/nlid_twoterm.h"
#include "netlist/solver/nld_solver.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(CD4053_GATE)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4053_GATE, "CD4XXX")
		, m_RX(*this, "RX")
		, m_RY(*this, "RY")
		, m_select(*this, "S", NETLIB_DELEGATE(controls))
		, m_inhibit(*this, "INH", NETLIB_DELEGATE(controls))
		, m_base_r(*this, "BASER", nlconst::magic(270.0))
		, m_lastx(*this, "m_lastx", false)
		, m_lasty(*this, "m_lasty", false)
		, m_select_state(*this, "m_select_state", false)
		, m_inhibit_state(*this, "m_inhibit_state", false)
		, m_supply(*this)
		{
			connect(m_RX.N(), m_RY.N());

			register_subalias("X", m_RX.P());
			register_subalias("Y", m_RY.P());

			register_subalias("XY", m_RX.N());
		}

		NETLIB_RESETI()
		{
			// Start in off condition
			// FIXME: is ROFF correct?
			m_RX.set_R(plib::reciprocal(exec().gmin()));
			m_RY.set_R(plib::reciprocal(exec().gmin()));
		}

	private:
		NETLIB_HANDLERI(controls)
		{
			bool newx = false, newy = false;
			if (!on(m_inhibit, m_inhibit_state))
			{
				if (!on(m_select, m_select_state))
				{
					newx = true;
				}
				else
				{
					newy = true;
				}
			}
			if (newx != m_lastx)
			{
				update_state(m_RX, newx);
				m_lastx = newx;
			}
			if (newy != m_lasty)
			{
				update_state(m_RY, newy);
				m_lasty = newy;
			}
		}

		bool on(analog_input_t &input, bool &state)
		{
			nl_fptype sup = (m_supply.VCC().Q_Analog() - m_supply.GND().Q_Analog());
			nl_fptype in = input() - m_supply.GND().Q_Analog();
			nl_fptype low = nlconst::magic(0.45) * sup;
			nl_fptype high = nlconst::magic(0.55) * sup;
			if (in < low)
			{
				state = false;
			}
			else if (in > high)
			{
				state = true;
			}
			return state;
		}

		void update_state(analog::NETLIB_SUB(R_base) &R, bool state)
		{
			nl_fptype Rval = plib::reciprocal(exec().gmin());
			if (state)
			{
				nl_fptype sup = (m_supply.VCC().Q_Analog() - m_supply.GND().Q_Analog());
				Rval = m_base_r() * nlconst::magic(5.0) / sup;
			}
			R.change_state([this, Rval]() -> void { this->m_RX.set_R(Rval);});
		}

		analog::NETLIB_SUB(R_base) m_RX;
		analog::NETLIB_SUB(R_base) m_RY;
		analog_input_t             m_select;
		analog_input_t             m_inhibit;
		param_fp_t                 m_base_r;
		state_var<bool>            m_lastx;
		state_var<bool>            m_lasty;
		state_var<bool>            m_select_state;
		state_var<bool>            m_inhibit_state;
		nld_power_pins             m_supply;
	};

	NETLIB_DEVICE_IMPL(CD4053_GATE,         "CD4053_GATE",            "")
	} //namespace devices
} // namespace netlist
