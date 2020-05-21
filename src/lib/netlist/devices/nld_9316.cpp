// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9316.c
 *
 */
#include "nld_9316.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{

	static constexpr const unsigned MAXCNT = 15;

	// FIXME: Family needs to be set to LS for 74193. This needs some more
	//        thought generally.

	template <bool ASYNC>
	NETLIB_OBJECT(9316_base)
	{
		NETLIB_CONSTRUCTOR(9316_base)
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(9316_base, clk))
		, m_ENT(*this, "ENT")
		, m_RC(*this, "RC")
		, m_LOADQ(*this, "LOADQ")
		, m_ENP(*this, "ENP")
		, m_CLRQ(*this, "CLRQ")
		, m_ABCD(*this, {"A", "B", "C", "D"}, NETLIB_DELEGATE(9316_base, abcd))
		, m_Q(*this, { "QA", "QB", "QC", "QD" })
		, m_cnt(*this, "m_cnt", 0)
		, m_abcd(*this, "m_abcd", 0)
		, m_loadq(*this, "m_loadq", 0)
		, m_ent(*this, "m_ent", 0)
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_CLK.set_state(logic_t::STATE_INP_LH);
			m_cnt = 0;
			m_abcd = 0;
		}

		NETLIB_UPDATEI()
		{
			const auto CLRQ(m_CLRQ());
			m_ent = m_ENT();
			m_loadq = m_LOADQ();

			if (((m_loadq ^ 1) || (m_ent && m_ENP())) && (!ASYNC || CLRQ))
			{
				m_CLK.activate_lh();
			}
			else
			{
				m_CLK.inactivate();
				if (ASYNC && !CLRQ && (m_cnt>0))
				{
					m_cnt = 0;
					m_Q.push(m_cnt, NLTIME_FROM_NS(36));
				}
			}
			m_RC.push(m_ent && (m_cnt == MAXCNT), NLTIME_FROM_NS(27));
		}


		NETLIB_HANDLERI(clk)
		{
			if (!ASYNC && !m_CLRQ())
			{
				m_cnt = 0;
				m_Q.push(m_cnt, NLTIME_FROM_NS(36));
			}
			else
			{
				//const auto cnt = (m_LOADQ() ? (m_cnt + 1) & MAXCNT: m_abcd);
				const auto cnt = (m_loadq ? (m_cnt + 1) & MAXCNT: m_abcd);
				m_RC.push(m_ent && (cnt == MAXCNT), NLTIME_FROM_NS(27));
				m_Q.push(cnt, NLTIME_FROM_NS(20));
				m_cnt = static_cast<unsigned>(cnt);
			}
		}

		NETLIB_HANDLERI(abcd)
		{
			m_abcd = static_cast<unsigned>(m_ABCD());
		}

		logic_input_t m_CLK;
		logic_input_t m_ENT;

		logic_output_t m_RC;

		logic_input_t m_LOADQ;

		logic_input_t m_ENP;
		logic_input_t m_CLRQ;

		object_array_t<logic_input_t, 4> m_ABCD;
		object_array_t<logic_output_t, 4> m_Q;

		/* counter state */
		state_var<unsigned> m_cnt;
		/* cached pins */
		state_var<unsigned> m_abcd;
		state_var_sig m_loadq;
		state_var_sig m_ent;
		nld_power_pins m_power_pins;

	};

	template <bool ASYNC>
	NETLIB_OBJECT(9316_dip_base)
	{
		NETLIB_CONSTRUCTOR(9316_dip_base)
		, A(*this, "A")
		{
			this->register_subalias("1", "A.CLRQ");
			this->register_subalias("2", "A.CLK");
			this->register_subalias("3", "A.A");
			this->register_subalias("4", "A.B");
			this->register_subalias("5", "A.C");
			this->register_subalias("6", "A.D");
			this->register_subalias("7", "A.ENP");
			this->register_subalias("8", "A.GND");

			this->register_subalias("9", "A.LOADQ");
			this->register_subalias("10", "A.ENT");
			this->register_subalias("11", "A.QD");
			this->register_subalias("12", "A.QC");
			this->register_subalias("13", "A.QB");
			this->register_subalias("14", "A.QA");
			this->register_subalias("15", "A.RC");
			this->register_subalias("16", "A.VCC");
		}
	private:
		NETLIB_SUB(9316_base)<ASYNC> A;
	};

	using NETLIB_NAME(9316) = NETLIB_NAME(9316_base)<true>;
	using NETLIB_NAME(74163) = NETLIB_NAME(9316_base)<false>;

	using NETLIB_NAME(9316_dip) = NETLIB_NAME(9316_dip_base)<true>;
	using NETLIB_NAME(74163_dip) = NETLIB_NAME(9316_dip_base)<false>;

	NETLIB_DEVICE_IMPL(9316,     "TTL_9316",     "+CLK,+ENP,+ENT,+CLRQ,+LOADQ,+A,+B,+C,+D,@VCC,@GND")
	NETLIB_DEVICE_IMPL(9316_dip, "TTL_9316_DIP", "")

	NETLIB_DEVICE_IMPL(74163,     "TTL_74163",     "+CLK,+ENP,+ENT,+CLRQ,+LOADQ,+A,+B,+C,+D,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74163_dip, "TTL_74163_DIP", "")

	} //namespace devices
} // namespace netlist
