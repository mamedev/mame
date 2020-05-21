// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9316.cpp
 *
 */
#include "nl_base.h"
#include "nl_factory.h"

namespace netlist
{
	namespace devices
	{

	// FIXME: Family needs to be set to LS for 74193. This needs some more
	//        thought generally.

	template <unsigned N>
	static constexpr unsigned rollover(unsigned v) { return v <= N ? v : 0; }

#if 0
	template <>
	constexpr unsigned rollover<7>(unsigned v) { return v & 7; }
#endif

	template <>
	constexpr unsigned rollover<15>(unsigned v) { return v & 15; }


	template <typename D>
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

			if (((m_loadq ^ 1) || (m_ent && m_ENP())) && (!D::ASYNC::value() || CLRQ))
			{
				m_CLK.activate_lh();
			}
			else
			{
				m_CLK.inactivate();
				if (D::ASYNC::value() && !CLRQ && (m_cnt>0))
				{
					m_cnt = 0;
					m_Q.push(m_cnt, D::tCLR::value(0));
				}
			}
			m_RC.push(m_ent && (m_cnt == D::MAXCNT::value()), D::tRC::value(0));
		}

		NETLIB_HANDLERI(clk)
		{
			if (!D::ASYNC::value() && !m_CLRQ())
			{
				m_cnt = 0;
				m_Q.push(m_cnt, D::tCLR::value(0));
			}
			else
			{
				//const auto cnt = (m_loadq ? (m_cnt + 1) & D::MAXCNT::value : m_abcd);
				const auto cnt = (m_loadq ? rollover<D::MAXCNT::value()>(m_cnt + 1) : m_abcd);
				m_RC.push(m_ent && (cnt == D::MAXCNT::value()), D::tRC::value(0));
				m_Q.push(cnt, D::tLDCNT::value(0));
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

	template <typename D>
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
		NETLIB_SUB(9316_base)<D> A;
	};

	struct desc_9316 : public desc_base
	{
		using ASYNC  = fdesc_const_t<bool, true>;
		using MAXCNT = fdesc_const_t<unsigned, 15>;
		using tRC    = time_ns<27>;
		using tCLR   = time_ns<36>;
		using tLDCNT = time_ns<20>;
	};

	struct desc_9310 : public desc_base
	{
		using ASYNC  = fdesc_const_t<bool, true>;
		using MAXCNT = fdesc_const_t<unsigned, 10>;
		using tRC    = time_ns<27>;
		using tCLR   = time_ns<36>;
		using tLDCNT = time_ns<20>;
	};

	struct desc_74161 : public desc_base
	{
		using ASYNC  = fdesc_const_t<bool, true>;
		using MAXCNT = fdesc_const_t<unsigned, 15>;
		using tRC    = time_ns<28>;
		using tCLR   = time_ns<37>;
		using tLDCNT = time_ns<21>;
	};

	struct desc_74163 : public desc_base
	{
		using ASYNC  = fdesc_const_t<bool, false>;
		using MAXCNT = fdesc_const_t<unsigned, 15>;
		using tRC    = time_ns<28>;
		using tCLR   = time_ns<37>;
		using tLDCNT = time_ns<21>;
	};

	using NETLIB_NAME(9310) = NETLIB_NAME(9316_base)<desc_9310>;
	using NETLIB_NAME(9316) = NETLIB_NAME(9316_base)<desc_9316>;
	using NETLIB_NAME(74161) = NETLIB_NAME(9316_base)<desc_74161>;
	using NETLIB_NAME(74161_fixme) = NETLIB_NAME(9316_base)<desc_74161>;
	using NETLIB_NAME(74163) = NETLIB_NAME(9316_base)<desc_74163>;

	using NETLIB_NAME(9310_dip) = NETLIB_NAME(9316_dip_base)<desc_9310>;
	using NETLIB_NAME(9316_dip) = NETLIB_NAME(9316_dip_base)<desc_9316>;
	using NETLIB_NAME(74161_dip) = NETLIB_NAME(9316_dip_base)<desc_74161>;
	using NETLIB_NAME(74163_dip) = NETLIB_NAME(9316_dip_base)<desc_74163>;

	NETLIB_DEVICE_IMPL(9310,     "TTL_9310",     "+CLK,+ENP,+ENT,+CLRQ,+LOADQ,+A,+B,+C,+D,@VCC,@GND")
	NETLIB_DEVICE_IMPL(9310_dip, "TTL_9310_DIP", "")

	NETLIB_DEVICE_IMPL(9316,     "TTL_9316",     "+CLK,+ENP,+ENT,+CLRQ,+LOADQ,+A,+B,+C,+D,@VCC,@GND")
	NETLIB_DEVICE_IMPL(9316_dip, "TTL_9316_DIP", "")

#if 1
	NETLIB_DEVICE_IMPL(74161,     "TTL_74161",     "+CLK,+ENP,+ENT,+CLRQ,+LOADQ,+A,+B,+C,+D,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74161_dip, "TTL_74161_DIP", "")

	// FIXME: This happens on copy/paste
	NETLIB_DEVICE_IMPL(74161_fixme, "TTL_74161_FIXME", "+A,+B,+C,+D,+CLRQ,+LOADQ,+CLK,+ENP,+ENT,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74163,     "TTL_74163",     "+CLK,+ENP,+ENT,+CLRQ,+LOADQ,+A,+B,+C,+D,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74163_dip, "TTL_74163_DIP", "")
#endif

	} //namespace devices
} // namespace netlist
