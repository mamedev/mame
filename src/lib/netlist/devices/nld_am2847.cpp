// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_am2847.cpp
 *
 */

#include "nld_am2847.h"
#include "../nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(Am2847_shifter)
	{
		NETLIB_CONSTRUCTOR(Am2847_shifter)
		, m_RC(*this, "RC")
		, m_IN(*this, "IN")
		, m_buffer(*this, "m_buffer", 0)
		, m_OUT(*this, "OUT")
		{
		}

		NETLIB_UPDATEI();

	public:
		void shift();

		logic_input_t m_RC;
		logic_input_t m_IN;

		state_array<uint16_t, 5> m_buffer;

		logic_output_t m_OUT;
	};

	NETLIB_OBJECT(AM2847)
	{
		NETLIB_CONSTRUCTOR(AM2847)
		, m_A(*this, "A")
		, m_B(*this, "B")
		, m_C(*this, "C")
		, m_D(*this, "D")
		, m_CP(*this, "CP")
		, m_last_CP(*this, "m_last_CP", 0)
		{
			register_subalias("OUTA", m_A.m_OUT);
			register_subalias("OUTB", m_B.m_OUT);
			register_subalias("OUTC", m_C.m_OUT);
			register_subalias("OUTD", m_D.m_OUT);
			register_subalias("INA", m_A.m_IN);
			register_subalias("INB", m_B.m_IN);
			register_subalias("INC", m_C.m_IN);
			register_subalias("IND", m_D.m_IN);
			register_subalias("RCA", m_A.m_RC);
			register_subalias("RCB", m_B.m_RC);
			register_subalias("RCC", m_C.m_RC);
			register_subalias("RCD", m_D.m_RC);
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		NETLIB_SUB(Am2847_shifter) m_A;
		NETLIB_SUB(Am2847_shifter) m_B;
		NETLIB_SUB(Am2847_shifter) m_C;
		NETLIB_SUB(Am2847_shifter) m_D;
		logic_input_t m_CP;

		state_var<uint32_t> m_last_CP;
	};

	NETLIB_OBJECT_DERIVED(AM2847_dip, AM2847)
	{
		NETLIB_CONSTRUCTOR_DERIVED(AM2847_dip, AM2847)
		{
			register_subalias("1", m_A.m_OUT);
			register_subalias("2", m_A.m_RC);
			register_subalias("3", m_A.m_IN);
			register_subalias("4", m_B.m_OUT);
			register_subalias("5", m_B.m_RC);
			register_subalias("6", m_B.m_IN);
			register_subalias("7", m_C.m_OUT);

			register_subalias("9",  m_C.m_RC);
			register_subalias("10", m_C.m_IN);
			register_subalias("11", m_CP);
			register_subalias("13", m_D.m_OUT);
			register_subalias("14", m_D.m_RC);
			register_subalias("15", m_D.m_IN);

		}
	};

	NETLIB_RESET(AM2847)
	{
		m_last_CP = 0;
	}

	NETLIB_UPDATE(AM2847)
	{
		if (m_last_CP && !m_CP())
		{
			m_A.shift();
			m_B.shift();
			m_C.shift();
			m_D.shift();
		}
		m_last_CP = m_CP();
	}

	NETLIB_UPDATE(Am2847_shifter)
	{
		/* do nothing */
	}

	NETLIB_FUNC_VOID(Am2847_shifter, shift, ())
	{
		uint32_t out = m_buffer[0] & 1;
		uint32_t in = (m_RC() ? out : m_IN());
		for (std::size_t i=0; i < 5; i++)
		{
			uint32_t shift_in = (i == 4) ? in : m_buffer[i + 1];
			m_buffer[i] >>= 1;
			m_buffer[i] |= shift_in << 15;
		}

		m_OUT.push(out, NLTIME_FROM_NS(200));
	}

	NETLIB_DEVICE_IMPL(AM2847,     "TTL_AM2847",     "+CP,+INA,+INB,+INC,+IND,+RCA,+RCB,+RCC,+RCD")
	NETLIB_DEVICE_IMPL(AM2847_dip, "TTL_AM2847_DIP", "")

	} //namespace devices
} // namespace netlist
