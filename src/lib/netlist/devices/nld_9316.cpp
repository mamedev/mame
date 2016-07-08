// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9316.c
 *
 */

#include "nld_9316.h"

#define MAXCNT 15

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(9316_subABCD)
	{
		NETLIB_CONSTRUCTOR(9316_subABCD)
		, m_A(*this, "A")
		, m_B(*this, "B")
		, m_C(*this, "C")
		, m_D(*this, "D")
		{
		}

		//NETLIB_RESETI()
		//NETLIB_UPDATEI();

	public:
		logic_input_t m_A;
		logic_input_t m_B;
		logic_input_t m_C;
		logic_input_t m_D;

		uint_fast8_t read_ABCD() const
		{
			//return (INPLOGIC_PASSIVE(m_D) << 3) | (INPLOGIC_PASSIVE(m_C) << 2) | (INPLOGIC_PASSIVE(m_B) << 1) | (INPLOGIC_PASSIVE(m_A) << 0);
			return (INPLOGIC(m_D) << 3) | (INPLOGIC(m_C) << 2) | (INPLOGIC(m_B) << 1) | (INPLOGIC(m_A) << 0);
		}
	};

	NETLIB_OBJECT(9316_sub)
	{
		NETLIB_CONSTRUCTOR(9316_sub)
		, m_CLK(*this, "CLK")
		, m_cnt(*this, "m_cnt", 0)
		, m_loadq(*this, "m_loadq", 0)
		, m_ent(*this, "m_ent", 0)
		, m_QA(*this, "QA")
		, m_QB(*this, "QB")
		, m_QC(*this, "QC")
		, m_QD(*this, "QD")
		, m_RC(*this, "RC")
		, m_ABCD(nullptr)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	public:
		void update_outputs_all(const uint_fast8_t cnt, const netlist_time out_delay)
		{
			OUTLOGIC(m_QA, (cnt >> 0) & 1, out_delay);
			OUTLOGIC(m_QB, (cnt >> 1) & 1, out_delay);
			OUTLOGIC(m_QC, (cnt >> 2) & 1, out_delay);
			OUTLOGIC(m_QD, (cnt >> 3) & 1, out_delay);
		}

		logic_input_t m_CLK;

		state_var_u8 m_cnt;
		state_var_u8 m_loadq;
		state_var_u8 m_ent;

		logic_output_t m_QA;
		logic_output_t m_QB;
		logic_output_t m_QC;
		logic_output_t m_QD;
		logic_output_t m_RC;

		NETLIB_NAME(9316_subABCD) *m_ABCD;
	};

	NETLIB_OBJECT(9316)
	{
		NETLIB_CONSTRUCTOR(9316)
		, sub(*this, "sub")
		, subABCD(*this, "subABCD")
		, m_ENP(*this, "ENP")
		, m_ENT(*this, "ENT")
		, m_CLRQ(*this, "CLRQ")
		, m_LOADQ(*this, "LOADQ")
		{
			sub.m_ABCD = &(subABCD);

			register_subalias("CLK", sub.m_CLK);

			register_subalias("A", subABCD.m_A);
			register_subalias("B", subABCD.m_B);
			register_subalias("C", subABCD.m_C);
			register_subalias("D", subABCD.m_D);

			register_subalias("QA", sub.m_QA);
			register_subalias("QB", sub.m_QB);
			register_subalias("QC", sub.m_QC);
			register_subalias("QD", sub.m_QD);
			register_subalias("RC", sub.m_RC);
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		NETLIB_SUB(9316_sub) sub;
		NETLIB_SUB(9316_subABCD) subABCD;
		logic_input_t m_ENP;
		logic_input_t m_ENT;
		logic_input_t m_CLRQ;
		logic_input_t m_LOADQ;
	};

	NETLIB_OBJECT_DERIVED(9316_dip, 9316)
	{
		NETLIB_CONSTRUCTOR_DERIVED(9316_dip, 9316)
		{
			register_subalias("1", m_CLRQ);
			register_subalias("2", sub.m_CLK);
			register_subalias("3", subABCD.m_A);
			register_subalias("4", subABCD.m_B);
			register_subalias("5", subABCD.m_C);
			register_subalias("6", subABCD.m_D);
			register_subalias("7", m_ENP);
			// register_subalias("8", ); -. GND

			register_subalias("9", m_LOADQ);
			register_subalias("10", m_ENT);
			register_subalias("11", sub.m_QD);
			register_subalias("12", sub.m_QC);
			register_subalias("13", sub.m_QB);
			register_subalias("14", sub.m_QA);
			register_subalias("15", sub.m_RC);
			// register_subalias("16", ); -. VCC
		}
	};

	NETLIB_RESET(9316)
	{
		sub.do_reset();
		subABCD.do_reset();
	}

	NETLIB_RESET(9316_sub)
	{
		m_CLK.set_state(logic_t::STATE_INP_LH);
		m_cnt = 0;
		m_loadq = 1;
		m_ent = 1;
	}

	NETLIB_UPDATE(9316_sub)
	{
		if (m_loadq)
		{
			switch (m_cnt())
			{
				case MAXCNT - 1:
					m_cnt = MAXCNT;
					OUTLOGIC(m_RC, m_ent, NLTIME_FROM_NS(27));
					OUTLOGIC(m_QA, 1, NLTIME_FROM_NS(20));
					break;
				case MAXCNT:
					OUTLOGIC(m_RC, 0, NLTIME_FROM_NS(27));
					m_cnt = 0;
					update_outputs_all(m_cnt, NLTIME_FROM_NS(20));
					break;
				default:
					m_cnt++;
					update_outputs_all(m_cnt, NLTIME_FROM_NS(20));
					break;
			}
		}
		else
		{
			m_cnt = m_ABCD->read_ABCD();
			OUTLOGIC(m_RC, m_ent & (m_cnt == MAXCNT), NLTIME_FROM_NS(27));
			update_outputs_all(m_cnt, NLTIME_FROM_NS(22));
		}
	}

	NETLIB_UPDATE(9316)
	{
		sub.m_loadq = INPLOGIC(m_LOADQ);
		sub.m_ent = INPLOGIC(m_ENT);
		const netlist_sig_t clrq = INPLOGIC(m_CLRQ);

		if (((sub.m_loadq ^ 1) | (sub.m_ent & INPLOGIC(m_ENP))) & clrq)
		{
			sub.m_CLK.activate_lh();
			OUTLOGIC(sub.m_RC, sub.m_ent & (sub.m_cnt == MAXCNT), NLTIME_FROM_NS(27));
		}
		else
		{
			sub.m_CLK.inactivate();
			if (!clrq && (sub.m_cnt>0))
			{
				sub.update_outputs_all(0, NLTIME_FROM_NS(36));
				sub.m_cnt = 0;
				//return;
			}
			OUTLOGIC(sub.m_RC, sub.m_ent & (sub.m_cnt == MAXCNT), NLTIME_FROM_NS(27));
		}
	}


	NETLIB_DEVICE_IMPL(9316)
	NETLIB_DEVICE_IMPL(9316_dip)

	} //namespace devices
} // namespace netlist
