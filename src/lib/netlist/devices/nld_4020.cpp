// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4020.c
 *
 */

#include "nld_4020.h"
#include "nl_base.h"
#include "nl_factory.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(CD4020_sub)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4020_sub, "CD4XXX")
		, m_IP(*this, "IP", NETLIB_DELEGATE(ip))
		, m_Q(*this, {"Q1", "_Q2", "_Q3", "Q4", "Q5", "Q6", "Q7", "Q8", "Q9",
				"Q10", "Q11", "Q12", "Q13", "Q14"})
		, m_cnt(*this, "m_cnt", 0)
		, m_supply(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_IP.set_state(logic_t::STATE_INP_HL);
			m_cnt = 0;
		}

		NETLIB_HANDLERI(ip)
		{
			++m_cnt;
			m_cnt &= 0x3fff;
			update_outputs(m_cnt);
		}

		NETLIB_UPDATEI()
		{
			ip();
		}

	public:
		void update_outputs(const unsigned cnt) noexcept
		{
			static constexpr const std::array<netlist_time, 14> out_delayQn = {
					NLTIME_FROM_NS(180), NLTIME_FROM_NS(280),
					NLTIME_FROM_NS(380), NLTIME_FROM_NS(480),
					NLTIME_FROM_NS(580), NLTIME_FROM_NS(680),
					NLTIME_FROM_NS(780), NLTIME_FROM_NS(880),
					NLTIME_FROM_NS(980), NLTIME_FROM_NS(1080),
					NLTIME_FROM_NS(1180), NLTIME_FROM_NS(1280),
					NLTIME_FROM_NS(1380), NLTIME_FROM_NS(1480),
			};

			m_Q[0].push(cnt & 1, out_delayQn[0]);
			for (std::size_t i=3; i<14; i++)
				m_Q[i].push((cnt >> i) & 1, out_delayQn[i]);
		}
		logic_input_t m_IP;
		object_array_t<logic_output_t, 14> m_Q;

		state_var<unsigned> m_cnt;
		nld_power_pins m_supply;
	};

	NETLIB_OBJECT(CD4020)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4020, "CD4XXX")
		, m_sub(*this, "sub")
		, m_RESET(*this, "RESET", NETLIB_DELEGATE(inputs))
		{
			register_subalias("IP", m_sub.m_IP);
			register_subalias("Q1", m_sub.m_Q[0]);
			register_subalias("Q4", m_sub.m_Q[3]);
			register_subalias("Q5", m_sub.m_Q[4]);
			register_subalias("Q6", m_sub.m_Q[5]);
			register_subalias("Q7", m_sub.m_Q[6]);
			register_subalias("Q8", m_sub.m_Q[7]);
			register_subalias("Q9", m_sub.m_Q[8]);
			register_subalias("Q10", m_sub.m_Q[9]);
			register_subalias("Q11", m_sub.m_Q[10]);
			register_subalias("Q12", m_sub.m_Q[11]);
			register_subalias("Q13", m_sub.m_Q[12]);
			register_subalias("Q14", m_sub.m_Q[13]);
			register_subalias("VDD", "sub.VDD");
			register_subalias("VSS", "sub.VSS");
		}

		NETLIB_RESETI() { }

		NETLIB_HANDLERI(inputs)
		{
			if (m_RESET())
			{
				m_sub.m_cnt = 0;
				m_sub.m_IP.inactivate();
				/* static */ const netlist_time reset_time = netlist_time::from_nsec(140);
				m_sub.m_Q[0].push(0, reset_time);
				for (std::size_t i=3; i<14; i++)
					m_sub.m_Q[i].push(0, reset_time);
			}
			else
				m_sub.m_IP.activate_hl();
		}

		NETLIB_UPDATEI()
		{
			inputs();
		}

	private:
		NETLIB_SUB(CD4020_sub) m_sub;
		logic_input_t m_RESET;
	};



	NETLIB_DEVICE_IMPL(CD4020,         "CD4020", "")
	NETLIB_DEVICE_IMPL_ALIAS(CD4020_WI, CD4020, "CD4020_WI", "+IP,+RESET,+VDD,+VSS")

	} //namespace devices
} // namespace netlist
