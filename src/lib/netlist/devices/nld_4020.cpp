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

	template <unsigned _LiveBitmask>
	NETLIB_OBJECT(CD4020_sub)
	{
		static constexpr unsigned MAX_BITS = 14;
		static constexpr unsigned MAX_BITMASK = (1 << MAX_BITS) - 1;

		NETLIB_CONSTRUCTOR_MODEL(CD4020_sub, "CD4XXX")
		, m_IP(*this, "IP", NETLIB_DELEGATE(ip))
		, m_RESET(*this, "RESET", NETLIB_DELEGATE(reseti))
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
			update_outputs(m_cnt);
		}

		NETLIB_HANDLERI(reseti)
		{
			if (m_RESET())
			{
				m_cnt = 0;
				m_IP.inactivate();
				/* static */ const netlist_time reset_time = netlist_time::from_nsec(140);
				for (int i = 0; i < MAX_BITS; i++)
					if (((_LiveBitmask >> i) & 1) != 0)
						m_Q[i].push(0, reset_time);
			}
			else
				m_IP.activate_hl();
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

			for (int i = 0; i < MAX_BITS; i++)
				if (((_LiveBitmask >> i) & 1) != 0)
					m_Q[i].push(cnt & 1, out_delayQn[i]);
		}
		logic_input_t m_IP;
		logic_input_t m_RESET;
		object_array_t<logic_output_t, MAX_BITS> m_Q;

		state_var<unsigned> m_cnt;
		nld_power_pins m_supply;
	};

	NETLIB_OBJECT(CD4020)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4020, "CD4XXX")
		, m_sub(*this, "sub")
		{
			register_subalias("IP", m_sub.m_IP);
			register_subalias("RESET", m_sub.m_RESET);
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

		//NETLIB_RESETI() {}

	private:
		NETLIB_SUB(CD4020_sub)<0x3ff9> m_sub;
	};

	NETLIB_OBJECT(CD4024)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4024, "CD4XXX")
		, m_sub(*this, "sub")
		{
			register_subalias("IP", m_sub.m_IP);
			register_subalias("RESET", m_sub.m_RESET);
			register_subalias("Q1", m_sub.m_Q[0]);
			register_subalias("Q2", m_sub.m_Q[1]);
			register_subalias("Q3", m_sub.m_Q[2]);
			register_subalias("Q4", m_sub.m_Q[3]);
			register_subalias("Q5", m_sub.m_Q[4]);
			register_subalias("Q6", m_sub.m_Q[5]);
			register_subalias("Q7", m_sub.m_Q[6]);
			register_subalias("VDD", "sub.VDD");
			register_subalias("VSS", "sub.VSS");
		}

		//NETLIB_RESETI() {}

	private:
		NETLIB_SUB(CD4020_sub)<0x7f> m_sub;
	};



	NETLIB_DEVICE_IMPL(CD4020,         "CD4020", "")
	NETLIB_DEVICE_IMPL_ALIAS(CD4020_WI, CD4020, "CD4020_WI", "+IP,+RESET,+VDD,+VSS")

	NETLIB_DEVICE_IMPL(CD4024,         "CD4024", "")

	} //namespace devices
} // namespace netlist
