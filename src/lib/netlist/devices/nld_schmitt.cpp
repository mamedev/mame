// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 * nld_schmitt.cpp
 *
 */

#include "nld_schmitt.h"

#include "netlist/analog/nlid_twoterm.h"
#include "netlist/nl_base.h"
#include "netlist/nl_errstr.h"
#include "netlist/solver/nld_solver.h"

#include <cmath>


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
				, m_GND(*this, "GND")
				, m_RVI(*this, "RVI")
				, m_RVO(*this, "RVO")
				, m_model(*this, "MODEL", "TTL_7414_GATE")
				, m_last_state(*this, "m_last_var", 1)
				, m_is_timestep(false)
			{
				register_subalias("Q", m_RVO.m_P);

				connect(m_A, m_RVI.m_P);
				connect(m_GND, m_RVI.m_N);
				connect(m_GND, m_RVO.m_N);
			}

		protected:
			NETLIB_RESETI()
			{
				m_last_state = 1;
				m_RVI.reset();
				m_RVO.reset();
				m_is_timestep = m_RVO.m_P.net().solver()->has_timestep_devices();
				m_RVI.set_G_V_I(plib::constants<nl_double>::one() / m_model.m_RI, m_model.m_VI, 0.0);
				m_RVO.set_G_V_I(plib::constants<nl_double>::one() / m_model.m_ROL, m_model.m_VOL, 0.0);
			}

			NETLIB_UPDATEI()
			{
				if (m_last_state)
				{
					if (m_A.Q_Analog() < m_model.m_VTM)
					{
						m_last_state = 0;
						if (m_is_timestep)
							m_RVO.update();
						m_RVO.set_G_V_I(plib::constants<nl_double>::one() / m_model.m_ROH, m_model.m_VOH, 0.0);
						m_RVO.solve_later();
					}
				}
				else
				{
					if (m_A.Q_Analog() > m_model.m_VTP)
					{
						m_last_state = 1;
						if (m_is_timestep)
							m_RVO.update();
						m_RVO.set_G_V_I(plib::constants<nl_double>::one() / m_model.m_ROL, m_model.m_VOL, 0.0);
						m_RVO.solve_later();
					}
				}
			}

		private:
			analog_input_t m_A;
			analog_input_t m_GND;
			analog::NETLIB_SUB(twoterm) m_RVI;
			analog::NETLIB_SUB(twoterm) m_RVO;
			schmitt_trigger_model_t m_model;
			state_var<int> m_last_state;
			bool m_is_timestep;
		};

		NETLIB_DEVICE_IMPL(schmitt_trigger, "SCHMITT_TRIGGER", "MODEL")

	} // namespace devices
} // namespace netlist
