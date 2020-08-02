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
		, m_A(*this, {"A0", "A1", "A2", "A3"}, NETLIB_DELEGATE(inputs))
		, m_B(*this, {"B0", "B1", "B2", "B3"}, NETLIB_DELEGATE(inputs))
		, m_LTIN(*this, "LTIN", NETLIB_DELEGATE(inputs))
		, m_EQIN(*this, "EQIN", NETLIB_DELEGATE(inputs))
		, m_GTIN(*this, "GTIN", NETLIB_DELEGATE(inputs))
		, m_LTOUT(*this, "LTOUT")
		, m_EQOUT(*this, "EQOUT")
		, m_GTOUT(*this, "GTOUT")
		, m_power_pins(*this)
		{
		}

	private:
		// FIXME: Timing
		NETLIB_HANDLERI(inputs)
		{
			for (std::size_t i = 4; i-- > 0; )
			{
				if (m_A[i]() > m_B[i]())
				{
					update_outputs(1, 0, 0);
					return;
				}

				if (m_A[i]() < m_B[i]())
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

		void update_outputs(unsigned gt, unsigned lt, unsigned eq)
		{
			m_GTOUT.push(gt, NLTIME_FROM_NS(23));
			m_LTOUT.push(lt, NLTIME_FROM_NS(23));
			m_EQOUT.push(eq, NLTIME_FROM_NS(23));
		}

		object_array_t<logic_input_t, 4> m_A;
		object_array_t<logic_input_t, 4> m_B;
		logic_input_t m_LTIN;
		logic_input_t m_EQIN;
		logic_input_t m_GTIN;
		logic_output_t m_LTOUT;
		logic_output_t m_EQOUT;
		logic_output_t m_GTOUT;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(7485, "TTL_7485", "+A0,+A1,+A2,+A3,+B0,+B1,+B2,+B3,+LTIN,+EQIN,+GTIN,@VCC,@GND")

	} //namespace devices
} // namespace netlist
