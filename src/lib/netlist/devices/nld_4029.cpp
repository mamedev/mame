// license:GPL-2.0+
// copyright-holders:Couriersud, Jonathan Gevaryahu
/*
 * nld_4029.cpp
 *
 *  CD4029BM/CD4029BC Presettable Binary/Decade Up/Down Counter
 *
 *      +--------------+
 *   PE |1     ++    16| VDD
 *   Q4 |2           15| CLK
 *   J4 |3           14| Q3
 *   J1 |4    4029   13| J3
 *   CI |5           12| J2
 *   Q1 |6           11| Q2
 *   CO |7           10| U/D
 *  VSS |8            9| B/D
 *      +--------------+
 *
 *          Counter Sequences:
 *
 *          B/D high (Binary), U/D high (Up)
 *          +-------++----+----+----+----+----++-------+
 *          | COUNT || Q4 | Q3 | Q2 | Q1 | CO ||  NEXT |
 *          +=======++====+====+====+====+====++=======+
 *          |    0  ||  0 |  0 |  0 |  0 |  1 ||     1 |
 *          |    1  ||  0 |  0 |  0 |  1 |  1 ||     2 |
 *          |    2  ||  0 |  0 |  1 |  0 |  1 ||     3 |
 *          |    3  ||  0 |  0 |  1 |  1 |  1 ||     4 |
 *          |    4  ||  0 |  1 |  0 |  0 |  1 ||     5 |
 *          |    5  ||  0 |  1 |  0 |  1 |  1 ||     6 |
 *          |    6  ||  0 |  1 |  1 |  0 |  1 ||     7 |
 *          |    7  ||  0 |  1 |  1 |  1 |  1 ||     8 |
 *          |    8  ||  1 |  0 |  0 |  0 |  1 ||     9 |
 *          |    9  ||  1 |  0 |  0 |  1 |  1 ||     A |
 *          |    A  ||  1 |  0 |  1 |  0 |  1 ||     B |
 *          |    B  ||  1 |  0 |  1 |  1 |  1 ||     C |
 *          |    C  ||  1 |  1 |  0 |  0 |  1 ||     D |
 *          |    D  ||  1 |  1 |  0 |  1 |  1 ||     E |
 *          |    E  ||  1 |  1 |  1 |  0 |  1 ||     F |
 *          |    F  ||  1 |  1 |  1 |  1 |0|CI||     0 |
 *          |    0  ||  0 |  0 |  0 |  0 |  1 ||     1 |
 *          +-------++----+----+----+----+----++-------+
 *
 *          B/D high (Binary), U/D low (Down)
 *          +-------++----+----+----+----+----++-------+
 *          | COUNT || Q4 | Q3 | Q2 | Q1 | CO ||  NEXT |
 *          +=======++====+====+====+====+====++=======+
 *          |    F  ||  1 |  1 |  1 |  1 |  1 ||     E |
 *          |    E  ||  1 |  1 |  1 |  0 |  1 ||     D |
 *          |    D  ||  1 |  1 |  0 |  1 |  1 ||     C |
 *          |    C  ||  1 |  1 |  0 |  0 |  1 ||     B |
 *          |    B  ||  1 |  0 |  1 |  1 |  1 ||     A |
 *          |    A  ||  1 |  0 |  1 |  0 |  1 ||     9 |
 *          |    9  ||  1 |  0 |  0 |  1 |  1 ||     8 |
 *          |    8  ||  1 |  0 |  0 |  0 |  1 ||     7 |
 *          |    7  ||  0 |  1 |  1 |  1 |  1 ||     6 |
 *          |    6  ||  0 |  1 |  1 |  0 |  1 ||     5 |
 *          |    5  ||  0 |  1 |  0 |  1 |  1 ||     4 |
 *          |    4  ||  0 |  1 |  0 |  0 |  1 ||     3 |
 *          |    3  ||  0 |  0 |  1 |  1 |  1 ||     2 |
 *          |    2  ||  0 |  0 |  1 |  0 |  1 ||     1 |
 *          |    1  ||  0 |  0 |  0 |  1 |  1 ||     0 |
 *          |    0  ||  0 |  0 |  0 |  0 |0|CI||     F |
 *          |    F  ||  1 |  1 |  1 |  1 |  1 ||     E |
 *          +-------++----+----+----+----+----++-------+
 *
 *          B/D low (Decimal), U/D high (Up)
 *          +-------++----+----+----+----+----++-------+
 *          | COUNT || Q4 | Q3 | Q2 | Q1 | CO ||  NEXT |
 *          +=======++====+====+====+====+====++=======+
 *          |    0  ||  0 |  0 |  0 |  0 |  1 ||     1 |
 *          |    1  ||  0 |  0 |  0 |  1 |  1 ||     2 |
 *          |    2  ||  0 |  0 |  1 |  0 |  1 ||     3 |
 *          |    3  ||  0 |  0 |  1 |  1 |  1 ||     4 |
 *          |    4  ||  0 |  1 |  0 |  0 |  1 ||     5 |
 *          |    5  ||  0 |  1 |  0 |  1 |  1 ||     6 |
 *          |    6  ||  0 |  1 |  1 |  0 |  1 ||     7 |
 *          |    7  ||  0 |  1 |  1 |  1 |  1 ||     8 |
 *          |    8  ||  1 |  0 |  0 |  0 |  1 ||     9 |
 *          |    9  ||  1 |  0 |  0 |  1 |0|CI||     0 |
 *          |    A  ||  1 |  0 |  1 |  0 |  1 ||     B |
 *          |    B  ||  1 |  0 |  1 |  1 |0|CI||     6 |
 *          |    C  ||  1 |  1 |  0 |  0 |  1 ||     D |
 *          |    D  ||  1 |  1 |  0 |  1 |0|CI||     4 |
 *          |    E  ||  1 |  1 |  1 |  0 |  1 ||     F |
 *          |    F  ||  1 |  1 |  1 |  1 |0|CI||     2 |
 *          |    0  ||  0 |  0 |  0 |  0 |  1 ||     1 |
 *          +-------++----+----+----+----+----++-------+
 *
 *          B/D low (Decimal), U/D low (Down)
 *          +-------++----+----+----+----+----++-------+
 *          | COUNT || Q4 | Q3 | Q2 | Q1 | CO ||  NEXT |
 *          +=======++====+====+====+====+====++=======+
 *          |    F  ||  1 |  1 |  1 |  1 |  1 ||     E |
 *          |    E  ||  1 |  1 |  1 |  0 |  1 ||     D |
 *          |    D  ||  1 |  1 |  0 |  1 |  1 ||     C |
 *          |    C  ||  1 |  1 |  0 |  0 |  1 ||     B |
 *          |    B  ||  1 |  0 |  1 |  1 |  1 ||     A |
 *          |    A  ||  1 |  0 |  1 |  0 |  1 ||     9 |
 *          |    9  ||  1 |  0 |  0 |  1 |  1 ||     8 |
 *          |    8  ||  1 |  0 |  0 |  0 |  1 ||     7 |
 *          |    7  ||  0 |  1 |  1 |  1 |  1 ||     6 |
 *          |    6  ||  0 |  1 |  1 |  0 |  1 ||     5 |
 *          |    5  ||  0 |  1 |  0 |  1 |  1 ||     4 |
 *          |    4  ||  0 |  1 |  0 |  0 |  1 ||     3 |
 *          |    3  ||  0 |  0 |  1 |  1 |  1 ||     2 |
 *          |    2  ||  0 |  0 |  1 |  0 |  1 ||     1 |
 *          |    1  ||  0 |  0 |  0 |  1 |  1 ||     0 |
 *          |    0  ||  0 |  0 |  0 |  0 |0|CI||     9 |
 *          |    9  ||  1 |  0 |  0 |  1 |  1 ||     8 |
 *          +-------++----+----+----+----+----++-------+
 *
 *          Note that in all cases where Carry-out is generating a non-1 signal (0 | Carry-in),
 *          this signal is asynchronous, so if carry-in changes the carry-out value will
 *          change immediately.
 *
 *
 *          Preset/Carry in function table
 *          +-----+-----+-----++----+----+----+----+
 *          | PE  | CLK | CI  || Q1 | Q2 | Q3 | Q4 |
 *          +=====+=====+=====++====+====+====+====+
 *          |  1  |  X  |  X  || J1 | J2 | J3 | J4 |
 *          |  0  |  X  |  1  || Q1 | Q2 | Q3 | Q4 |
 *          |  0  | ./` |  0  ||       COUNT       |
 *          +-----+-----+-----++----+----+----+----+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#include "nl_base.h"

namespace netlist::devices {

	NETLIB_OBJECT(CD4029)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4029, "CD4XXX")
		, m_PE(*this, "PE", NETLIB_DELEGATE(inputs))
		, m_J(*this, {"J1", "J2", "J3", "J4"}, NETLIB_DELEGATE(inputs))
		, m_CI(*this, "CI", NETLIB_DELEGATE(inputs))
		, m_UD(*this, "UD", NETLIB_DELEGATE(inputs))
		, m_BD(*this, "BD", NETLIB_DELEGATE(inputs))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_clk_old(*this, "m_clk_old", false)
		, m_Q(*this, {"Q1", "Q2", "Q3", "Q4"})
		, m_CO(*this, "CO", false)
		, m_cnt(*this, "m_cnt", 0)
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_HANDLERI(inputs) // pe causes an asynchronous counter load on level so has to be handled separately; if pe is high, then J changing will asynchronously change the Q state. changing J in this state does affect CO as well; CI will affect CO asynchronously if there is currently a carry
		{
			if (m_PE())
			{
				m_cnt = m_J()&0xf;
			}
			m_Q.push(m_cnt, NLTIME_FROM_NS(200));
			// figure out CO
			if (!m_UD()) // downward is the same for binary and decimal
			{
				m_CO.push(m_cnt||m_CI(),NLTIME_FROM_NS(320));
			}
			else // upward mode differs
			{
				if (!m_BD()) // decimal mode
				{
					m_CO.push((m_cnt<8)||(~m_cnt&1)||m_CI(),NLTIME_FROM_NS(320));
				}
				else // binary mode
				{
					m_CO.push((m_cnt!=0xf)||m_CI(),NLTIME_FROM_NS(320));
				}
			}
		}

		NETLIB_HANDLERI(clk)
		{
			// clocking only happens if m_clk_old was low, m_CLK is high, m_PE is NOT high, and m_CI is NOT high.
			if (!m_PE() && !m_CI() && !m_clk_old && m_CLK())
			{
				if (m_BD()) // binary
				{
					m_cnt += (m_UD() ? 1 : -1);
					m_cnt &= 0xf;
				}
				else // decimal
				{
					if (m_UD()) // upward
					{
						switch(m_cnt)
						{
							case 0: case 1: case 2: case 3:
							case 4: case 5: case 6: case 7:
							case 8: case 0xa: case 0xc: case 0xe:
								m_cnt++;
								break;
							case 9:
								m_cnt = 0;
								break;
							case 0xb:
								m_cnt = 6;
								break;
							case 0xd:
								m_cnt = 4;
								break;
							case 0xf:
								m_cnt = 2;
								break;
						}
					}
					else // downward
					{
						if (m_cnt>0)
						{
							m_cnt--;
						}
						else // m_cnt == 0
						{
							m_cnt = 9;
						}
					}
				}
			}
			m_clk_old = m_CLK();
			m_Q.push(m_cnt, NLTIME_FROM_NS(200));
			// figure out CO
			if (!m_UD()) // downward is the same for binary and decimal
			{
				m_CO.push(m_cnt||m_CI(),NLTIME_FROM_NS(320));
			}
			else // upward mode differs
			{
				if (!m_BD()) // decimal mode
				{
					m_CO.push((m_cnt<8)||(~m_cnt&1)||m_CI(),NLTIME_FROM_NS(320));
				}
				else // binary mode
				{
					m_CO.push((m_cnt!=0xf)||m_CI(),NLTIME_FROM_NS(320));
				}
			}
		}

		NETLIB_RESETI()
		{
			m_cnt = 0;
			m_clk_old = false;
		}

		logic_input_t m_PE;
		object_array_t<logic_input_t, 4> m_J;
		logic_input_t m_CI;
		logic_input_t m_UD;
		logic_input_t m_BD;
		logic_input_t m_CLK;

		state_var<netlist_sig_t> m_clk_old;
		object_array_t<logic_output_t, 4> m_Q;
		logic_output_t m_CO;
		state_var_u8 m_cnt;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(CD4029,     "CD4029",        "+PE,+J1,+J2,+J3,+J4,+CI,+UD,+BD,+CLK,@VCC,@GND")

} // namespace netlist::devices
