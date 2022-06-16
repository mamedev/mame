// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 * nld_schmitt.cpp
 *
 */

#include "analog/nlid_twoterm.h"
#include "nl_base.h"
#include "nl_errstr.h"
#include "solver/nld_solver.h"

namespace netlist::devices {

	/*
	 * Over-simplified TTL Schmitt trigger model
	 * It's good enough to work for 74xx/74LSxx if it isn't abused too badly
	 * It would need major changes to support 4000 or 74HCxx behaviour
	 * The input is over-simplified - it's actually more like a resistor in series with a diode, and can only source current
	 * Similarly, high output can only source current and low output can only sink current
	 * Propagation delays are ignored
	 *
	 */

	class schmitt_trigger_model_t
	{
	public:
		schmitt_trigger_model_t(param_model_t &model)
			: m_VTP(model, "VTP")
			, m_VTM(model, "VTM")
			, m_VI(model, "VI")
			, m_RI(model, "RI")
			, m_VOH(model, "VOH")
			, m_ROH(model, "ROH")
			, m_VOL(model, "VOL")
			, m_ROL(model, "ROL")
			, m_TPLH(model, "TPLH")
			, m_TPHL(model, "TPHL")
		{
		}

		param_model_t::value_t m_VTP;
		param_model_t::value_t m_VTM;
		param_model_t::value_t m_VI;
		param_model_t::value_t m_RI;
		param_model_t::value_t m_VOH;
		param_model_t::value_t m_ROH;
		param_model_t::value_t m_VOL;
		param_model_t::value_t m_ROL;
		param_model_t::value_t m_TPLH;
		param_model_t::value_t m_TPHL;
	};

	NETLIB_OBJECT(schmitt_trigger)
	{
		NETLIB_CONSTRUCTOR(schmitt_trigger)
			, m_A(*this, "A", NETLIB_DELEGATE(input))
			, m_supply(*this)
			, m_RVI(*this, "RVI")
			, m_RVO(*this, "RVO")
			, m_stmodel(*this, "STMODEL", "TTL_7414_GATE")
			, m_modacc(m_stmodel)
			, m_last_state(*this, "m_last_var", 1)
		{
			register_sub_alias("Q", "RVO.1");

			connect("A", "RVI.1");
			// FIXME: need a symbolic reference from connect as well
			connect(m_supply.GND(), m_RVI.N());
			connect(m_supply.GND(), m_RVO.N());
		}

	protected:
		NETLIB_RESETI()
		{
			m_last_state = 1;
			m_RVI.reset();
			m_RVO.reset();
			m_RVI.set_G_V_I(plib::reciprocal(m_modacc.m_RI()), m_modacc.m_VI, nlconst::zero());
			m_RVO.set_G_V_I(plib::reciprocal(m_modacc.m_ROL()), m_modacc.m_VOL, nlconst::zero());
		}

	private:
		NETLIB_HANDLERI(input)
		{
			const auto va(m_A.Q_Analog() - m_supply.GND().Q_Analog());
			if (m_last_state)
			{
				if (va < m_modacc.m_VTM)
				{
					m_last_state = 0;
					m_RVO.change_state([this]()
					{
						m_RVO.set_G_V_I(plib::reciprocal(m_modacc.m_ROH()), m_modacc.m_VOH, nlconst::zero());
					});
				}
			}
			else
			{
				if (va > m_modacc.m_VTP)
				{
					m_last_state = 1;
					m_RVO.change_state([this]()
					{
						m_RVO.set_G_V_I(plib::reciprocal(m_modacc.m_ROL()), m_modacc.m_VOL, nlconst::zero());
					});
				}
			}
		}

		analog_input_t m_A;
		NETLIB_NAME(power_pins) m_supply;
		analog::NETLIB_SUB(two_terminal) m_RVI;
		analog::NETLIB_SUB(two_terminal) m_RVO;
		param_model_t m_stmodel;
		schmitt_trigger_model_t m_modacc;
		state_var<int> m_last_state;
	};

	NETLIB_DEVICE_IMPL(schmitt_trigger, "SCHMITT_TRIGGER", "STMODEL")

} // namespace netlist::devices
