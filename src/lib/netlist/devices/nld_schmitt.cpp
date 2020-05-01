// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 * nld_schmitt.cpp
 *
 */

#include "nld_schmitt.h"

#include "netlist/analog/nlid_twoterm.h"
#include "netlist/devices/nlid_system.h"
#include "netlist/nl_base.h"
#include "netlist/nl_errstr.h"
#include "netlist/solver/nld_solver.h"

namespace netlist
{
	namespace devices
	{

		/*
		 * Over-simplified TTL Schmitt trigger model
		 * It's good enough to work for 74xx/74LSxx if it isn't abused too badly
		 * It would need major changes to support 4000 or 74HCxx behaviour
		 * The input is over-simplified - it's actually more like a resistor in series with a diode, and can only source current
		 * Similarly, high output can only source current and low output can only sink current
		 * Propagation delays are ignored
		 *
		 */

		class schmitt_trigger_model_t : public param_model_t
		{
		public:
			schmitt_trigger_model_t(device_t &device, const pstring &name, const pstring &val)
				: param_model_t(device, name, val)
				, m_VTP(*this, "VTP")
				, m_VTM(*this, "VTM")
				, m_VI(*this, "VI")
				, m_RI(*this, "RI")
				, m_VOH(*this, "VOH")
				, m_ROH(*this, "ROH")
				, m_VOL(*this, "VOL")
				, m_ROL(*this, "ROL")
				, m_TPLH(*this, "TPLH")
				, m_TPHL(*this, "TPHL")
			{
			}

			value_t m_VTP;
			value_t m_VTM;
			value_t m_VI;
			value_t m_RI;
			value_t m_VOH;
			value_t m_ROH;
			value_t m_VOL;
			value_t m_ROL;
			value_t m_TPLH;
			value_t m_TPHL;
		};

		NETLIB_OBJECT(schmitt_trigger)
		{
			NETLIB_CONSTRUCTOR(schmitt_trigger)
				, m_A(*this, "A")
				, m_supply(*this)
				, m_RVI(*this, "RVI")
				, m_RVO(*this, "RVO")
				, m_model(*this, "MODEL", "TTL_7414_GATE")
				, m_last_state(*this, "m_last_var", 1)
			{
				register_subalias("Q", m_RVO.P());

				connect(m_A, m_RVI.P());
				connect(m_supply.GND(), m_RVI.N());
				connect(m_supply.GND(), m_RVO.N());
			}

		protected:
			NETLIB_RESETI()
			{
				m_last_state = 1;
				m_RVI.reset();
				m_RVO.reset();
				m_RVI.set_G_V_I(plib::reciprocal(m_model.m_RI()), m_model.m_VI, nlconst::zero());
				m_RVO.set_G_V_I(plib::reciprocal(m_model.m_ROL()), m_model.m_VOL, nlconst::zero());
			}

			NETLIB_UPDATEI()
			{
				const auto va(m_A.Q_Analog() - m_supply.GND().Q_Analog());
				if (m_last_state)
				{
					if (va < m_model.m_VTM)
					{
						m_last_state = 0;
						m_RVO.change_state([this]()
						{
							m_RVO.set_G_V_I(plib::reciprocal(m_model.m_ROH()), m_model.m_VOH, nlconst::zero());
						});
					}
				}
				else
				{
					if (va > m_model.m_VTP)
					{
						m_last_state = 1;
						m_RVO.change_state([this]()
						{
							m_RVO.set_G_V_I(plib::reciprocal(m_model.m_ROL()), m_model.m_VOL, nlconst::zero());
						});
					}
				}
			}

		private:
			analog_input_t m_A;
			NETLIB_NAME(power_pins) m_supply;
			analog::NETLIB_SUB(twoterm) m_RVI;
			analog::NETLIB_SUB(twoterm) m_RVO;
			schmitt_trigger_model_t m_model;
			state_var<int> m_last_state;
		};

		NETLIB_DEVICE_IMPL(schmitt_trigger, "SCHMITT_TRIGGER", "MODEL")

	} // namespace devices
} // namespace netlist
