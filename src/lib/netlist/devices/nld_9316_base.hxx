// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_9316_base.hxx
 *
 */

#include "nl_base.h"
#include "nl_factory.h"

namespace netlist::devices
{

	template <unsigned N>
	static constexpr unsigned rollover(unsigned v) noexcept { return v <= N ? v : 0; }

	template <>
	constexpr unsigned rollover<15>(unsigned v) noexcept { return v & 15; }

	template <typename D>
	NETLIB_OBJECT(9316_base)
	{
		NETLIB_CONSTRUCTOR(9316_base)
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_ENT(*this, "ENT", NETLIB_DELEGATE(other))
		, m_LOADQ(*this, "LOADQ", NETLIB_DELEGATE(other))
		, m_ENP(*this, "ENP", NETLIB_DELEGATE(other))
		, m_CLRQ(*this, "CLRQ", NETLIB_DELEGATE(other))
		, m_ABCD(*this, {"A", "B", "C", "D"}, NETLIB_DELEGATE(abcd))
		, m_RC(*this, "RC")
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

		NETLIB_HANDLERI(other)
		{
			const auto CLRQ(m_CLRQ());
			m_ent = m_ENT();
			m_loadq = m_LOADQ();

			if (((m_loadq ^ 1) || (m_ent && m_ENP())) && (!D::ASYNC::value || CLRQ))
			{
				m_CLK.activate_lh();
			}
			else
			{
				m_CLK.inactivate();
				if (D::ASYNC::value && !CLRQ && (m_cnt>0))
				{
					m_cnt = 0;
					m_Q.push(0, D::tCLR::value(0));
				}
			}
			m_RC.push(m_ent && (m_cnt == D::MAXCNT::value), D::tRC::value(0));
		}

		NETLIB_HANDLERI(clk)
		{
			if (!D::ASYNC::value && !m_CLRQ())
			{
				m_Q.push(0, D::tCLR::value(0));
				m_cnt = 0;
			}
			else
			{
				const unsigned cnt(m_loadq ? rollover<D::MAXCNT::value>(m_cnt + 1) : m_abcd);
				m_RC.push(m_ent && (cnt == D::MAXCNT::value), D::tRC::value(0));
				m_Q.push(cnt, D::tLDCNT::value(0));
				m_cnt = cnt;
			}
		}

		NETLIB_HANDLERI(abcd)
		{
			m_abcd = static_cast<unsigned>(m_ABCD());
		}

		logic_input_t m_CLK;
		logic_input_t m_ENT;

		logic_input_t m_LOADQ;

		logic_input_t m_ENP;
		logic_input_t m_CLRQ;

		object_array_t<logic_input_t, 4> m_ABCD;

		logic_output_t m_RC;
		object_array_t<logic_output_t, 4> m_Q;

		/* counter state */
		state_var<unsigned> m_cnt;
		/* cached pins */
		state_var<unsigned> m_abcd;
		state_var<unsigned> m_loadq;
		state_var<unsigned> m_ent;
		nld_power_pins m_power_pins;

	};
	struct desc_9316 : public desc_base
	{
		using ASYNC  = desc_const_t<bool, true>;
		using MAXCNT = desc_const_t<unsigned, 15>;
		using tRC    = time_ns<27>;
		using tCLR   = time_ns<36>;
		using tLDCNT = time_ns<20>;
	};

	struct desc_9310 : public desc_base
	{
		using ASYNC  = desc_const_t<bool, true>;
		using MAXCNT = desc_const_t<unsigned, 10>;
		using tRC    = time_ns<27>;
		using tCLR   = time_ns<36>;
		using tLDCNT = time_ns<20>;
	};

	struct desc_74161 : public desc_base
	{
		using ASYNC  = desc_const_t<bool, true>;
		using MAXCNT = desc_const_t<unsigned, 15>;
		using tRC    = time_ns<27>;
		using tCLR   = time_ns<36>;
		using tLDCNT = time_ns<20>;
	};

	struct desc_74163 : public desc_base
	{
		using ASYNC  = desc_const_t<bool, false>;
		using MAXCNT = desc_const_t<unsigned, 15>;
		using tRC    = time_ns<27>;
		using tCLR   = time_ns<36>;
		using tLDCNT = time_ns<20>;
	};

	using NETLIB_NAME(9310) = NETLIB_NAME(9316_base)<desc_9310>;
	using NETLIB_NAME(9316) = NETLIB_NAME(9316_base)<desc_9316>;
	using NETLIB_NAME(74161) = NETLIB_NAME(9316_base)<desc_74161>;
	using NETLIB_NAME(74161_fixme) = NETLIB_NAME(9316_base)<desc_74161>;
	using NETLIB_NAME(74163) = NETLIB_NAME(9316_base)<desc_74163>;

} // namespace netlist::devices
