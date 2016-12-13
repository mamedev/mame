// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74107.c
 *
 */

#include "nld_74107.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(7473_flipflop_base)
	{
		NETLIB_CONSTRUCTOR(7473_flipflop_base)
		, m_CLK(*this, "CLK")
		, m_CLR(*this, "CLR")
		, m_J(*this, "J")
		, m_K(*this, "K")
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_last_CLK(*this, "m_last_CLK", 0)
		{
		}

		NETLIB_RESETI();

	public:
		logic_input_t m_CLK;
		logic_input_t m_CLR;
		logic_input_t m_J;
		logic_input_t m_K;

		logic_output_t m_Q;
		logic_output_t m_QQ;

		state_var<netlist_sig_t> m_last_CLK;

		void tick();
	};

	NETLIB_RESET(7473_flipflop_base)
	{
		m_last_CLK = 0;
	}

	inline NETLIB_FUNC_VOID(7473_flipflop_base, tick, (void))
	{
		const netlist_time delay[2] = { NLTIME_FROM_NS(40), NLTIME_FROM_NS(25) };

		const netlist_sig_t j = m_J();
		const netlist_sig_t k = m_K();
		const netlist_sig_t old_q = m_Q.net().Q();
		netlist_sig_t q = old_q;
		if (j && k)
		{
			q ^= 1;
		}
		else if (j)
		{
			q = 1;
		}
		else if (k)
		{
			q = 0;
		}

		if (q != old_q)
		{
			m_Q.push(q, delay[q]);
			m_QQ.push(q ^ 1, delay[q ^ 1]);
		}
	}

	NETLIB_OBJECT_DERIVED(7473_flipflop, 7473_flipflop_base)
	{
	public:
		NETLIB_CONSTRUCTOR_DERIVED(7473_flipflop, 7473_flipflop_base) { }
		NETLIB_UPDATEI();
	};

	NETLIB_UPDATE(7473_flipflop)
	{
		if (m_CLR())
		{
			m_Q.push(0, NLTIME_FROM_NS(40));
			m_QQ.push(1, NLTIME_FROM_NS(25));
		}
		else if (m_CLK())
		{
			tick();
		}

		m_last_CLK = m_CLK();
	}

	NETLIB_OBJECT_DERIVED(7473A_flipflop, 7473_flipflop_base)
	{
	public:
		NETLIB_CONSTRUCTOR_DERIVED(7473A_flipflop, 7473_flipflop_base) { }
		NETLIB_UPDATEI();
	};

	NETLIB_UPDATE(7473A_flipflop)
	{
		if (m_CLR())
		{
			m_Q.push(0, NLTIME_FROM_NS(40));
			m_QQ.push(1, NLTIME_FROM_NS(25));
		}
		else if (!m_CLK() && m_last_CLK)
		{
			tick();
		}

		m_last_CLK = m_CLK();
	};

	NETLIB_OBJECT(7473)
	{
		NETLIB_CONSTRUCTOR(7473)
		, m_FF1(*this, "FF1")
		, m_FF2(*this, "FF2")
		{ }

	protected:
		NETLIB_SUB(7473_flipflop) m_FF1;
		NETLIB_SUB(7473_flipflop) m_FF2;
	};

	NETLIB_OBJECT(7473A)
	{
		NETLIB_CONSTRUCTOR(7473A)
		, m_FF1(*this, "FF1")
		, m_FF2(*this, "FF2")
		{ }

	protected:
		NETLIB_SUB(7473A_flipflop) m_FF1;
		NETLIB_SUB(7473A_flipflop) m_FF2;
	};

	NETLIB_OBJECT_DERIVED(7473_dip, 7473)
	{
		NETLIB_CONSTRUCTOR_DERIVED(7473_dip, 7473)
		{
			register_subalias("1", m_FF1.m_CLK);
			register_subalias("2", m_FF1.m_CLR);
			register_subalias("3", m_FF1.m_K);
			// register_subalias("4", ); ==> VCC
			register_subalias("5", m_FF2.m_CLK);
			register_subalias("6", m_FF2.m_CLR);
			register_subalias("7", m_FF2.m_J);

			register_subalias("8",  m_FF2.m_QQ);
			register_subalias("9",  m_FF2.m_Q);
			register_subalias("10", m_FF2.m_K);
			// register_subalias("11", ); ==> GND
			register_subalias("12", m_FF1.m_Q);
			register_subalias("13", m_FF1.m_QQ);
			register_subalias("14", m_FF1.m_J);
		}
	};

	NETLIB_OBJECT_DERIVED(7473A_dip, 7473A)
	{
		NETLIB_CONSTRUCTOR_DERIVED(7473A_dip, 7473A)
		{
			register_subalias("1", m_FF1.m_CLK);
			register_subalias("2", m_FF1.m_CLR);
			register_subalias("3", m_FF1.m_K);
			// register_subalias("4", ); ==> VCC
			register_subalias("5", m_FF2.m_CLK);
			register_subalias("6", m_FF2.m_CLR);
			register_subalias("7", m_FF2.m_J);

			register_subalias("8",  m_FF2.m_QQ);
			register_subalias("9",  m_FF2.m_Q);
			register_subalias("10", m_FF2.m_K);
			// register_subalias("11", ); ==> GND
			register_subalias("12", m_FF1.m_Q);
			register_subalias("13", m_FF1.m_QQ);
			register_subalias("14", m_FF1.m_J);
		}
		//NETLIB_RESETI();
		//NETLIB_UPDATEI();

	private:
	};

	NETLIB_DEVICE_IMPL(7473)
	NETLIB_DEVICE_IMPL(7473A)
	NETLIB_DEVICE_IMPL(7473_dip)
	NETLIB_DEVICE_IMPL(7473A_dip)

	} //namespace devices
} // namespace netlist
