// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_7485.cpp
 *
 */

#include "nld_7485.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(7485)
	{
		NETLIB_CONSTRUCTOR(7485)
		, m_A(*this, {{"A0", "A1", "A2", "A3"}})
		, m_B(*this, {{"B0", "B1", "B2", "B3"}})
		, m_LTIN(*this, "LTIN")
		, m_EQIN(*this, "EQIN")
		, m_GTIN(*this, "GTIN")
		, m_LTOUT(*this, "LTOUT")
		, m_EQOUT(*this, "EQOUT")
		, m_GTOUT(*this, "GTOUT")
		{
		}

		NETLIB_UPDATEI();

		void update_outputs(unsigned gt, unsigned lt, unsigned eq);

	protected:
		object_array_t<logic_input_t, 4> m_A;
		object_array_t<logic_input_t, 4> m_B;
		logic_input_t m_LTIN;
		logic_input_t m_EQIN;
		logic_input_t m_GTIN;
		logic_output_t m_LTOUT;
		logic_output_t m_EQOUT;
		logic_output_t m_GTOUT;
	};

	NETLIB_OBJECT_DERIVED(7485_dip, 7485)
	{
		NETLIB_CONSTRUCTOR_DERIVED(7485_dip, 7485)
		{
			register_subalias("1", m_B[3]);
			register_subalias("2", m_LTIN);
			register_subalias("3", m_EQIN);
			register_subalias("4", m_GTIN);
			register_subalias("5", m_GTOUT);
			register_subalias("6", m_EQOUT);
			register_subalias("7", m_LTOUT);

			register_subalias("9",  m_B[0]);
			register_subalias("10", m_A[0]);
			register_subalias("11", m_B[1]);
			register_subalias("12", m_A[1]);
			register_subalias("13", m_A[2]);
			register_subalias("14", m_B[2]);
			register_subalias("15", m_A[3]);

		}
	};

	void NETLIB_NAME(7485)::update_outputs(unsigned gt, unsigned lt, unsigned eq)
	{
		m_GTOUT.push(gt, NLTIME_FROM_NS(23));
		m_LTOUT.push(lt, NLTIME_FROM_NS(23));
		m_EQOUT.push(eq, NLTIME_FROM_NS(23));
	}

	// FIXME: Timing
	NETLIB_UPDATE(7485)
	{
		for (std::size_t i = 4; i-- > 0; )
		{
			if (m_A[i]() > m_B[i]())
			{
				update_outputs(1, 0, 0);
				return;
			}
			else if (m_A[i]() < m_B[i]())
			{
				update_outputs(0, 1, 0);
				return;
			}
		}

		// must be == if we got here
		if (m_EQIN())
			update_outputs(0, 0, 1);
		else if (m_GTIN() && m_LTIN())
			update_outputs(0, 0, 0);
		else if (m_GTIN())
			update_outputs(1, 0, 0);
		else if (m_LTIN())
			update_outputs(0, 1, 0);
		else
			update_outputs(1, 1, 0);
	}

	NETLIB_DEVICE_IMPL(7485, "TTL_7485", "+A0,+A1,+A2,+A3,+B0,+B1,+B2,+B3,+LTIN,+EQIN,+GTIN")
	NETLIB_DEVICE_IMPL(7485_dip, "TTL_7485_DIP", "")

	} //namespace devices
} // namespace netlist
