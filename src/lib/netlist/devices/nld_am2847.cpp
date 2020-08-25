// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_am2847.cpp
 *
 *  Am2847: Quad 80-Bit Static Shift Register
 *
 *          +--------------+
 *     OUTA |1     ++    16| VSS
 *      RCA |2           15| IND
 *      INA |3           14| RCD
 *     OUTB |4   Am2847  13| OUTD
 *      RCB |5           12| VGG
 *      INB |6           11| CP
 *     OUTC |7           10| INC
 *      VDD |8            9| RCC
 *          +--------------+
 *
 */

#include "nld_am2847.h"
#include "nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(Am2847_shifter)
	{
		NETLIB_CONSTRUCTOR_MODEL(Am2847_shifter, "CD4XXX")
		, m_RC(*this, "RC", NETLIB_DELEGATE(inputs))
		, m_IN(*this, "IN", NETLIB_DELEGATE(inputs))
		, m_buffer(*this, "m_buffer", 0)
		, m_OUT(*this, "OUT")
		, m_power_pins(*this)
		{
		}

	public:
		void shift() noexcept
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

		logic_input_t m_RC;
		logic_input_t m_IN;

		state_container<std::array<uint16_t, 5>> m_buffer;

		logic_output_t m_OUT;
		nld_power_pins m_power_pins;
	private:
		NETLIB_HANDLERI(inputs)
		{
			/* do nothing */
		}
	};

	NETLIB_OBJECT(AM2847)
	{
		NETLIB_CONSTRUCTOR(AM2847)
		, m_A(*this, "A")
		, m_B(*this, "B")
		, m_C(*this, "C")
		, m_D(*this, "D")
		, m_CP(*this, "CP", NETLIB_DELEGATE(cp))
		, m_last_CP(*this, "m_last_CP", 0)
		// FIXME: needs family!
		{
			register_subalias("OUTA", "A.OUT");
			register_subalias("OUTB", "B.OUT");
			register_subalias("OUTC", "C.OUT");
			register_subalias("OUTD", "D.OUT");
			register_subalias("INA", "A.IN");
			register_subalias("INB", "B.IN");
			register_subalias("INC", "C.IN");
			register_subalias("IND", "D.IN");
			register_subalias("RCA", "A.RC");
			register_subalias("RCB", "B.RC");
			register_subalias("RCC", "C.RC");
			register_subalias("RCD", "D.RC");

			connect("A.VSS", "B.VSS");
			connect("A.VSS", "C.VSS");
			connect("A.VSS", "D.VSS");
			connect("A.VDD", "B.VDD");
			connect("A.VDD", "C.VDD");
			connect("A.VDD", "D.VDD");

			register_subalias("VSS", "A.VSS");
			register_subalias("VDD", "A.VDD");
		}

		NETLIB_RESETI()
		{
			m_last_CP = 0;
		}

	private:
		NETLIB_HANDLERI(cp)
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

		NETLIB_SUB(Am2847_shifter) m_A;
		NETLIB_SUB(Am2847_shifter) m_B;
		NETLIB_SUB(Am2847_shifter) m_C;
		NETLIB_SUB(Am2847_shifter) m_D;
		logic_input_t m_CP;

		state_var<uint32_t> m_last_CP;
	};

	NETLIB_DEVICE_IMPL(AM2847,     "TTL_AM2847",     "+CP,+INA,+INB,+INC,+IND,+RCA,+RCB,+RCC,+RCD,@VSS,@VDD")

	} //namespace devices
} // namespace netlist
