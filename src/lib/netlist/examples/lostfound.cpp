///
/// \file lostfound.cpp
///
/// This file contains various code removed from the core which may be
/// useful again in the future or serve educational purposes.
///
/// Don't expect this file to compile


/// \brief  Use the truthtable implementation of 74107 instead of the coded device
///
/// FIXME: The truthtable implementation of 74107 (JK-Flipflop)
///        is included for educational purposes to demonstrate how
///        to implement state holding devices as truthtables.
///        It will completely nuke performance for pong.

#ifndef NL_USE_TRUTHTABLE_74107
#define NL_USE_TRUTHTABLE_74107 (0)
#endif

#if (NL_USE_TRUTHTABLE_74107)
	/*
	 *          +-----+-----+-----+---++---+-----+
	 *          | CLRQ| CLK |  J  | K || Q | QQ  |
	 *          +=====+=====+=====+===++===+=====+
	 *          |  0  |  X  |  X  | X || 0 |  1  |
	 *          |  1  |  *  |  0  | 0 || Q0| Q0Q |
	 *          |  1  |  *  |  1  | 0 || 1 |  0  |
	 *          |  1  |  *  |  0  | 1 || 0 |  1  |
	 *          |  1  |  *  |  1  | 1 || TOGGLE  |
	 *          +-----+-----+-----+---++---+-----+
	 */
	TRUTHTABLE_START(TTL_74107, 6, 4, "+CLK,+J,+K,+CLRQ,@VCC,@GND")
		TT_HEAD("CLRQ, CLK, _CO,  J, K,_QX | Q, QQ, CO, QX")
		TT_LINE("  0,   0,    X,  X, X,  X | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  0,   1,    X,  X, X,  X | 0,  1,  1,  0 | 16, 25, 1, 1")

		TT_LINE("  1,   0,    X,  0, 0,  0 | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   1,    X,  0, 0,  0 | 0,  1,  1,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   0,    X,  0, 0,  1 | 1,  0,  0,  1 | 25, 16, 1, 1")
		TT_LINE("  1,   1,    X,  0, 0,  1 | 1,  0,  1,  1 | 25, 16, 1, 1")

		TT_LINE("  1,   0,    1,  1, 0,  X | 1,  0,  0,  1 | 25, 16, 1, 1")
		TT_LINE("  1,   0,    0,  1, 0,  0 | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   0,    0,  1, 0,  1 | 1,  0,  0,  1 | 25, 16, 1, 1")
		TT_LINE("  1,   1,    X,  1, 0,  0 | 0,  1,  1,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   1,    X,  1, 0,  1 | 1,  0,  1,  1 | 25, 16, 1, 1")

		TT_LINE("  1,   0,    1,  0, 1,  X | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   0,    0,  0, 1,  0 | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   0,    0,  0, 1,  1 | 1,  0,  0,  1 | 25, 16, 1, 1")
		TT_LINE("  1,   1,    X,  0, 1,  0 | 0,  1,  1,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   1,    X,  0, 1,  1 | 1,  0,  1,  1 | 25, 16, 1, 1")

		// Toggle
		TT_LINE("  1,   0,    0,  1, 1,  0 | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   0,    0,  1, 1,  1 | 1,  0,  0,  1 | 25, 16, 1, 1")
		TT_LINE("  1,   1,    0,  1, 1,  0 | 0,  1,  1,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   1,    0,  1, 1,  1 | 1,  0,  1,  1 | 25, 16, 1, 1")
		TT_LINE("  1,   1,    1,  1, 1,  0 | 0,  1,  1,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   1,    1,  1, 1,  1 | 1,  0,  1,  1 | 25, 16, 1, 1")

		TT_LINE("  1,   0,    1,  1, 1,  1 | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   0,    1,  1, 1,  0 | 1,  0,  0,  1 | 25, 16, 1, 1")
	TRUTHTABLE_END()
#endif

/// \brief  Use the truthtable implementation of 7448 instead of the coded device
///
/// FIXME: Using truthtable is a lot slower than the explicit device
///        in breakout. Performance drops by 20%. This can be fixed by
///        setting param USE_DEACTIVATE for the device.

#ifndef NL_USE_TRUTHTABLE_7448
#define NL_USE_TRUTHTABLE_7448 (0)
#endif

#if (NL_USE_TRUTHTABLE_7448)
	TRUTHTABLE_START(TTL_7448, 7, 7, "+A,+B,+C,+D,+LTQ,+BIQ,+RBIQ,@VCC,@GND")
		TT_HEAD(" LTQ,BIQ,RBIQ, A , B , C , D | a, b, c, d, e, f, g")

		TT_LINE("  1,  1,  1,   0,  0,  0,  0 | 1, 1, 1, 1, 1, 1, 0|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  0,  0 | 0, 1, 1, 0, 0, 0, 0|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  0,  0 | 1, 1, 0, 1, 1, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  0,  0 | 1, 1, 1, 1, 0, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  0,  1,  0 | 0, 1, 1, 0, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  1,  0 | 1, 0, 1, 1, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  1,  0 | 0, 0, 1, 1, 1, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  1,  0 | 1, 1, 1, 0, 0, 0, 0|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  0,  0,  1 | 1, 1, 1, 1, 1, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  0,  1 | 1, 1, 1, 0, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  0,  1 | 0, 0, 0, 1, 1, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  0,  1 | 0, 0, 1, 1, 0, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  0,  1,  1 | 0, 1, 0, 0, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  1,  1 | 1, 0, 0, 1, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  1,  1 | 0, 0, 0, 1, 1, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  1,  1 | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100")

		// BI/RBO is input output. In the next case it is used as an input will go low.
		TT_LINE("  1,  1,  0,   0,  0,  0,  0 | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100") // RBI

		TT_LINE("  0,  1,  X,   X,  X,  X,  X | 1, 1, 1, 1, 1, 1, 1|100,100,100,100,100,100,100") // LT

		// This condition has precedence
		TT_LINE("  X,  0,  X,   X,  X,  X,  X | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100") // BI
		TT_FAMILY("74XX")

	TRUTHTABLE_END()

	// FIXME: We need a more elegant solution than defining twice
	TRUTHTABLE_START(TTL_7448_TT, 7, 7, "")
		TT_HEAD(" LTQ,BIQ,RBIQ, A , B , C , D | a, b, c, d, e, f, g")

		TT_LINE("  1,  1,  1,   0,  0,  0,  0 | 1, 1, 1, 1, 1, 1, 0|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  0,  0 | 0, 1, 1, 0, 0, 0, 0|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  0,  0 | 1, 1, 0, 1, 1, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  0,  0 | 1, 1, 1, 1, 0, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  0,  1,  0 | 0, 1, 1, 0, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  1,  0 | 1, 0, 1, 1, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  1,  0 | 0, 0, 1, 1, 1, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  1,  0 | 1, 1, 1, 0, 0, 0, 0|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  0,  0,  1 | 1, 1, 1, 1, 1, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  0,  1 | 1, 1, 1, 0, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  0,  1 | 0, 0, 0, 1, 1, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  0,  1 | 0, 0, 1, 1, 0, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  0,  1,  1 | 0, 1, 0, 0, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  1,  1 | 1, 0, 0, 1, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  1,  1 | 0, 0, 0, 1, 1, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  1,  1 | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100")

		// BI/RBO is input output. In the next case it is used as an input will go low.
		TT_LINE("  1,  1,  0,   0,  0,  0,  0 | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100") // RBI

		TT_LINE("  0,  1,  X,   X,  X,  X,  X | 1, 1, 1, 1, 1, 1, 1|100,100,100,100,100,100,100") // LT

		// This condition has precedence
		TT_LINE("  X,  0,  X,   X,  X,  X,  X | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100") // BI
		TT_FAMILY("74XX")

	TRUTHTABLE_END()

#endif

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
		register_sub_alias("CTL", m_DUM1.P());

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

//  NETLIB_DEVICE_IMPL(CD4066_GATE_DYNAMIC, "CD4066_GATE_DYNAMIC",            "")

