// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9310.c
 *
 */

#include "nld_9310.h"
#include "netlist/nl_base.h"
#include "nlid_system.h"

#define MAXCNT 9

namespace netlist
{
	namespace devices
	{

	NETLIB_OBJECT(9310)
	{
		NETLIB_CONSTRUCTOR(9310)
		, m_ENP(*this, "ENP")
		, m_ENT(*this, "ENT")
		, m_CLRQ(*this, "CLRQ")
		, m_LOADQ(*this, "LOADQ")
		, m_A(*this, "A", NETLIB_DELEGATE(9310, abcd))
		, m_B(*this, "B", NETLIB_DELEGATE(9310, abcd))
		, m_C(*this, "C", NETLIB_DELEGATE(9310, abcd))
		, m_D(*this, "D", NETLIB_DELEGATE(9310, abcd))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(9310, sub))
		, m_QA(*this, "QA")
		, m_QB(*this, "QB")
		, m_QC(*this, "QC")
		, m_QD(*this, "QD")
		, m_RC(*this, "RC")
		, m_cnt(*this, "m_cnt", 0)
		, m_loadq(*this, "m_loadq", 0)
		, m_ent(*this, "m_ent", 0)
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

		NETLIB_HANDLERI(abcd) { } // do nothing

		NETLIB_HANDLERI(sub)
		{
			auto cnt(m_cnt);

			if (m_loadq)
			{
				if (cnt < MAXCNT - 1)
				{
					++cnt;
					update_outputs(cnt);
				}
				else if (cnt == MAXCNT - 1)
				{
					cnt = MAXCNT;
					m_RC.push(m_ent, NLTIME_FROM_NS(20));
					m_QA.push(1, NLTIME_FROM_NS(20));
				}
				else // MAXCNT
				{
					m_RC.push(0, NLTIME_FROM_NS(20));
					cnt = 0;
					update_outputs_all(cnt, NLTIME_FROM_NS(20));
				}
			}
			else
			{
				cnt = read_ABCD();
				m_RC.push(m_ent & (cnt == MAXCNT), NLTIME_FROM_NS(27));
				update_outputs_all(cnt, NLTIME_FROM_NS(22));
			}
			m_cnt = cnt;
		}

	protected:

		unsigned read_ABCD() const
		{
			return (m_D() << 3) | (m_C() << 2) | (m_B() << 1) | (m_A() << 0);
		}

		void update_outputs_all(const unsigned cnt, const netlist_time out_delay)
		{
			m_QA.push((cnt >> 0) & 1, out_delay);
			m_QB.push((cnt >> 1) & 1, out_delay);
			m_QC.push((cnt >> 2) & 1, out_delay);
			m_QD.push((cnt >> 3) & 1, out_delay);
		}

		void update_outputs(const unsigned cnt)
		{
			/* static */ const netlist_time out_delay = NLTIME_FROM_NS(20);
		#if 0
		//    for (int i=0; i<4; i++)
		//        m_Q[i], (cnt >> i) & 1, delay[i]);
			m_QA.push((cnt >> 0) & 1, out_delay);
			m_QB.push((cnt >> 1) & 1, out_delay);
			m_QC.push((cnt >> 2) & 1, out_delay);
			m_QD.push((cnt >> 3) & 1, out_delay);
		#else
			if ((cnt & 1) == 1)
				m_QA.push(1, out_delay);
			else
			{
				m_QA.push(0, out_delay);
				switch (cnt)
				{
				case 0x00:
					m_QB.push(0, out_delay);
					m_QC.push(0, out_delay);
					m_QD.push(0, out_delay);
					break;
				case 0x02:
				case 0x06:
				case 0x0A:
				case 0x0E:
					m_QB.push(1, out_delay);
					break;
				case 0x04:
				case 0x0C:
					m_QB.push(0, out_delay);
					m_QC.push(1, out_delay);
					break;
				case 0x08:
					m_QB.push(0, out_delay);
					m_QC.push(0, out_delay);
					m_QD.push(1, out_delay);
					break;
				}

			}
		#endif
		}

		logic_input_t m_ENP;
		logic_input_t m_ENT;
		logic_input_t m_CLRQ;
		logic_input_t m_LOADQ;
		logic_input_t m_A;
		logic_input_t m_B;
		logic_input_t m_C;
		logic_input_t m_D;
		logic_input_t m_CLK;

		logic_output_t m_QA;
		logic_output_t m_QB;
		logic_output_t m_QC;
		logic_output_t m_QD;
		logic_output_t m_RC;

		state_var<unsigned> m_cnt;
		state_var<netlist_sig_t> m_loadq;
		state_var<netlist_sig_t> m_ent;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT_DERIVED(9310_dip, 9310)
	{
		NETLIB_CONSTRUCTOR_DERIVED(9310_dip, 9310)
		{
			register_subalias("1", m_CLRQ);
			register_subalias("2", m_CLK);
			register_subalias("3", m_A);
			register_subalias("4", m_B);
			register_subalias("5", m_C);
			register_subalias("6", m_D);
			register_subalias("7", m_ENP);
			register_subalias("8", "GND");

			register_subalias("9", m_LOADQ);
			register_subalias("10", m_ENT);
			register_subalias("11", m_QD);
			register_subalias("12", m_QC);
			register_subalias("13", m_QB);
			register_subalias("14", m_QA);
			register_subalias("15", m_RC);
			register_subalias("16", "VCC");
		}
	};

	NETLIB_RESET(9310)
	{
		m_CLK.set_state(logic_t::STATE_INP_LH);
		m_cnt = 0;
		m_loadq = 1;
		m_ent = 1;
	}

	NETLIB_UPDATE(9310)
	{
		m_loadq = m_LOADQ();
		m_ent = m_ENT();
		const netlist_sig_t clrq = m_CLRQ();

		if ((!m_loadq || (m_ent & m_ENP())) && clrq)
		{
			m_CLK.activate_lh();
			m_RC.push(m_ent & (m_cnt == MAXCNT), NLTIME_FROM_NS(27));
		}
		else
		{
			m_CLK.inactivate();
			if (!clrq && (m_cnt>0))
			{
				update_outputs_all(0, NLTIME_FROM_NS(36));
				m_cnt = 0;
				//return;
			}
			m_RC.push(m_ent & (m_cnt == MAXCNT), NLTIME_FROM_NS(27));
		}
	}

	NETLIB_DEVICE_IMPL(9310,     "TTL_9310",     "+CLK,+ENP,+ENT,+CLRQ,+LOADQ,+A,+B,+C,+D,@VCC,@GND")
	NETLIB_DEVICE_IMPL(9310_dip, "TTL_9310_DIP", "")

	} //namespace devices
} // namespace netlist
