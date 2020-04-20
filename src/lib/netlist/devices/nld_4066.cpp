// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4066.c
 *
 */

#include "nld_4066.h"

#include "netlist/analog/nlid_twoterm.h"
#include "netlist/solver/nld_solver.h"
#include "nlid_system.h"

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
		NETLIB_CONSTRUCTOR(CD4066_GATE)
		NETLIB_FAMILY("CD4XXX")
		, m_supply(*this, "VDD", "VSS")
		, m_R(*this, "R")
		, m_control(*this, "CTL")
		, m_base_r(*this, "BASER", nlconst::magic(270.0))
		, m_last(*this, "m_last", false)
		{
		}

		NETLIB_RESETI()
		{
			// Start in off condition
			// FIXME: is ROFF correct?
			m_R.set_R(plib::reciprocal(exec().gmin()));
		}

		NETLIB_UPDATEI()
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
				m_R.update();
				m_R.set_R(R);
				m_R.solve_later();
			}
		}

	private:
		nld_power_pins             m_supply;
		analog::NETLIB_SUB(R_base) m_R;
		analog_input_t             m_control;
		param_fp_t                 m_base_r;
		state_var<bool>            m_last;
	};


	NETLIB_OBJECT(CD4066_GATE_DYNAMIC)
	{
		NETLIB_CONSTRUCTOR(CD4066_GATE_DYNAMIC)
		NETLIB_FAMILY("CD4XXX")
		, m_supply(*this, "VDD", "VSS")
		, m_R(*this, "R", true)
		, m_DUM1(*this, "_DUM1", true)
		, m_DUM2(*this, "_DUM2", true)
		, m_base_r(*this, "BASER", nlconst::magic(270.0))
		, m_last(*this, "m_last", false)
		{
			register_subalias("CTL", m_DUM1.m_P);   // Cathode

			connect(m_DUM1.m_P, m_DUM2.m_P);
			connect(m_DUM1.m_N, m_R.m_P);
			connect(m_DUM2.m_N, m_R.m_N);
		}

		NETLIB_RESETI()
		{
			// Start in off condition
			// FIXME: is ROFF correct?
		}

		NETLIB_UPDATEI()
		{
		}

		NETLIB_UPDATE_TERMINALSI()
		{
			nl_fptype sup = (m_supply.VCC().Q_Analog() - m_supply.GND().Q_Analog());
			nl_fptype in = m_DUM1.m_P.net().Q_Analog() - m_supply.GND().Q_Analog();
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
		nld_power_pins             m_supply;
		analog::nld_twoterm        m_R;
		analog::nld_twoterm        m_DUM1;
		analog::nld_twoterm        m_DUM2;
		param_fp_t                 m_base_r;
		state_var<bool>            m_last;
	};

#if !USE_DYNAMIC_APPROACH
	NETLIB_DEVICE_IMPL(CD4066_GATE,         "CD4066_GATE",            "")
#else
	NETLIB_DEVICE_IMPL_ALIAS(CD4066_GATE, CD4066_GATE_DYNAMIC,         "CD4066_GATE",            "")
#endif
	} //namespace devices
} // namespace netlist
