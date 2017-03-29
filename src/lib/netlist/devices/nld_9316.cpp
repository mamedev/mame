// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9316.c
 *
 */

#include "nld_9316.h"
#include "../nl_base.h"

#define MAXCNT 15

namespace netlist
{
	namespace devices
	{

	NETLIB_OBJECT(9316)
	{
		NETLIB_CONSTRUCTOR(9316)
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(9316, clk))
		, m_cnt(*this, "m_cnt", 0)
		, m_ENP(*this, "ENP")
		, m_ENT(*this, "ENT")
		, m_CLRQ(*this, "CLRQ")
		, m_LOADQ(*this, "LOADQ")
		, m_A(*this, "A", NETLIB_DELEGATE(9316, noop))
		, m_B(*this, "B", NETLIB_DELEGATE(9316, noop))
		, m_C(*this, "C", NETLIB_DELEGATE(9316, noop))
		, m_D(*this, "D", NETLIB_DELEGATE(9316, noop))
		, m_QA(*this, "QA")
		, m_QB(*this, "QB")
		, m_QC(*this, "QC")
		, m_QD(*this, "QD")
		, m_RC(*this, "RC")
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();
		NETLIB_HANDLERI(clk);
		NETLIB_HANDLERI(noop) { }

	protected:
		logic_input_t m_CLK;

		state_var<unsigned> m_cnt;

		logic_input_t m_ENP;
		logic_input_t m_ENT;
		logic_input_t m_CLRQ;
		logic_input_t m_LOADQ;

		logic_input_t m_A;
		logic_input_t m_B;
		logic_input_t m_C;
		logic_input_t m_D;

		logic_output_t m_QA;
		logic_output_t m_QB;
		logic_output_t m_QC;
		logic_output_t m_QD;
		logic_output_t m_RC;


	private:
		void update_outputs_all(const unsigned &cnt, const netlist_time &out_delay)
		{
			m_QA.push((cnt >> 0) & 1, out_delay);
			m_QB.push((cnt >> 1) & 1, out_delay);
			m_QC.push((cnt >> 2) & 1, out_delay);
			m_QD.push((cnt >> 3) & 1, out_delay);
		}
	};

	NETLIB_OBJECT_DERIVED(9316_dip, 9316)
	{
		NETLIB_CONSTRUCTOR_DERIVED(9316_dip, 9316)
		{
			register_subalias("1", m_CLRQ);
			register_subalias("2", m_CLK);
			register_subalias("3", m_A);
			register_subalias("4", m_B);
			register_subalias("5", m_C);
			register_subalias("6", m_D);
			register_subalias("7", m_ENP);
			// register_subalias("8", ); -. GND

			register_subalias("9", m_LOADQ);
			register_subalias("10", m_ENT);
			register_subalias("11", m_QD);
			register_subalias("12", m_QC);
			register_subalias("13", m_QB);
			register_subalias("14", m_QA);
			register_subalias("15", m_RC);
			// register_subalias("16", ); -. VCC
		}
	};

	NETLIB_RESET(9316)
	{
		m_CLK.set_state(logic_t::STATE_INP_LH);
		m_cnt = 0;
	}

	NETLIB_HANDLER(9316, clk)
	{
		if (m_LOADQ())
		{
			m_cnt = (m_cnt < MAXCNT ? m_cnt + 1 : 0);
			update_outputs_all(m_cnt, NLTIME_FROM_NS(20));
			m_RC.push(m_ENT() & (m_cnt == MAXCNT), NLTIME_FROM_NS(27));
		}
		else
		{
			m_cnt = (m_D() << 3) | (m_C() << 2) | (m_B() << 1) | (m_A() << 0);
			m_RC.push(m_ENT() & (m_cnt == MAXCNT), NLTIME_FROM_NS(27));
			update_outputs_all(m_cnt, NLTIME_FROM_NS(22));
		}
	}

	NETLIB_UPDATE(9316)
	{
		const netlist_sig_t clrq = m_CLRQ();

		if (((m_LOADQ() ^ 1) | (m_ENT() & m_ENP())) & clrq)
		{
			m_CLK.activate_lh();
			m_RC.push(m_ENT() & (m_cnt == MAXCNT), NLTIME_FROM_NS(27));
		}
		else
		{
			m_CLK.inactivate();
			if (!clrq && (m_cnt>0))
			{
				update_outputs_all(0, NLTIME_FROM_NS(36));
				m_cnt = 0;
			}
			m_RC.push(m_ENT() & (m_cnt == MAXCNT), NLTIME_FROM_NS(27));
		}
	}


	NETLIB_DEVICE_IMPL(9316)
	NETLIB_DEVICE_IMPL(9316_dip)

	} //namespace devices
} // namespace netlist
