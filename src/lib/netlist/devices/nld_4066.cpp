// license:GPL-2.0+
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

#include "nld_4066.h"

#include "netlist/analog/nlid_twoterm.h"
#include "netlist/solver/nld_solver.h"

// This is an experimental approach to implement the analog switch.
// This will make the switch a 3 terminal element which is completely
// being dealt with as part as the linear system.
//
// The intention was to improve convergence when the switch is in a feedback
// loop. One example are two-opamp tridiagonal wave generators.
// Unfortunately the approach did not work out and in addition was performing
// far worse than the net-separating original code.
//
// FIXME: The transfer function needs review
//

#define USE_DYNAMIC_APPROACH (0)

namespace netlist
{
	namespace devices
	{
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
			m_R.set_R(plib::reciprocal(exec().gmin()));
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
				m_R.change_state([this, &R]() -> void { this->m_R.set_R(R);});
			}
		}

		analog::NETLIB_SUB(R_base) m_R;
		analog_input_t             m_control;
		param_fp_t                 m_base_r;
		state_var<bool>            m_last;
		nld_power_pins             m_supply;
	};


	NETLIB_OBJECT(CD4066_GATE_DYNAMIC)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4066_GATE_DYNAMIC, "CD4XXX")
		, m_R(*this, "R", NETLIB_DELEGATE(analog_input_changed))
		, m_DUM1(*this, "_DUM1", NETLIB_DELEGATE(analog_input_changed))
		, m_DUM2(*this, "_DUM2", NETLIB_DELEGATE(analog_input_changed))
		, m_base_r(*this, "BASER", nlconst::magic(270.0))
		, m_last(*this, "m_last", false)
		, m_supply(*this)
		{
			register_subalias("CTL", m_DUM1.P());

			connect(m_DUM1.P(), m_DUM2.P());
			connect(m_DUM1.N(), m_R.P());
			connect(m_DUM2.N(), m_R.N());
		}

		NETLIB_RESETI()
		{
			// Start in off condition
			// FIXME: is ROFF correct?
		}

		NETLIB_UPDATE_TERMINALSI()
		{
			nl_fptype sup = (m_supply.VCC().Q_Analog() - m_supply.GND().Q_Analog());
			nl_fptype in = m_DUM1.P().net().Q_Analog() - m_supply.GND().Q_Analog();
			nl_fptype rON = m_base_r() * nlconst::magic(5.0) / sup;
			nl_fptype R = std::exp(-(in / sup - nlconst::magic(0.55)) * nlconst::magic(25.0)) + rON;
			nl_fptype G = plib::reciprocal(R);
			// dI/dVin = (VR1-VR2)*(1.0/sup*b) * exp((Vin/sup-a) * b)
			const auto dfdz = nlconst::magic(25.0)/(R*sup) * m_R.deltaV();
			const auto Ieq = dfdz * in;
			const auto zero(nlconst::zero());
			m_R.set_mat( G, -G, zero,
						-G,  G, zero);
						 //VIN  VR1
			m_DUM1.set_mat( zero,  zero,  zero,   // IIN
							dfdz,  zero,  Ieq);  // IR1
			m_DUM2.set_mat( zero,  zero,  zero,   // IIN
						   -dfdz,  zero, -Ieq);  // IR2
		}
		NETLIB_IS_DYNAMIC(true)

	private:

		NETLIB_HANDLERI(analog_input_changed)
		{
			m_R.solve_now();
		}

		analog::nld_twoterm        m_R;
		analog::nld_twoterm        m_DUM1;
		analog::nld_twoterm        m_DUM2;
		param_fp_t                 m_base_r;
		state_var<bool>            m_last;
		nld_power_pins             m_supply;
	};

#if !USE_DYNAMIC_APPROACH
	NETLIB_DEVICE_IMPL(CD4066_GATE,         "CD4066_GATE",            "")
#else
	NETLIB_DEVICE_IMPL_ALIAS(CD4066_GATE, CD4066_GATE_DYNAMIC,         "CD4066_GATE",            "")
#endif
	} //namespace devices
} // namespace netlist
