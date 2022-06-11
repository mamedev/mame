// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_74107.cpp
 *
 *  DM74107: DUAL J-K FLIP-FLOPS WITH CLEAR
 *
 *          +--------------+
 *       1J |1     ++    14| VCC
 *      1QQ |2           13| 1CLRQ
 *       1Q |3           12| 1CLK
 *       1K |4    74107  11| 2K
 *       2Q |5           10| 2CLRQ
 *      2QQ |6            9| 2CLK
 *      GND |7            8| 2J
 *          +--------------+
 *
 *
 *          Function table 107
 *
 *          +-----+-----+-----+---++---+-----+
 *          | CLRQ| CLK |  J  | K || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  0  |  X  |  X  | X || 0 |  1  |
 *          |  1  |  *  |  0  | 0 || Q0| Q0Q |
 *          |  1  |  *  |  1  | 0 || 1 |  0  |
 *          |  1  |  *  |  0  | 1 || 0 |  1  |
 *          |  1  |  *  |  1  | 1 || TOGGLE  |
 *          +-----+-----+-----+---++---+-----+
 *                _
 *          * = _| |_
 *
 *          This is positive triggered, J and K
 *          are latched during clock high and
 *          transferred when CLK falls. The
 *          datasheet requires J and K to be
 *          stable during clock high.
 *
 *          Function table 107A
 *
 *          +-----+-----+-----+---++---+-----+
 *          | CLRQ| CLK |  J  | K || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  0  |  X  |  X  | X || 0 |  1  |
 *          |  1  |  F  |  0  | 0 || Q0| Q0Q |
 *          |  1  |  F  |  1  | 0 || 1 |  0  |
 *          |  1  |  F  |  0  | 1 || 0 |  1  |
 *          |  1  |  F  |  1  | 1 || TOGGLE  |
 *          |  1  |  1  |  X  | X || Q0| Q0Q |
 *          +-----+-----+-----+---++---+-----+
 *
 *          THe 107A is negative triggered.
 *
 *  Naming conventions follow Texas instruments datasheet
 *
 *  TODO:  Currently, only the 107A is implemented.
 *         The 107 uses the same model, but different timings.
 *         The requirement that J and K must be stable during
 *         clock high indicates that the chip may exhibit undefined
 *         behaviour.
 *
 */

#include "nl_base.h"

namespace netlist::devices {

	template <typename D>
	NETLIB_OBJECT(74107_base)
	{
		NETLIB_CONSTRUCTOR(74107_base)
		, m_clk(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_J(*this, "J", NETLIB_DELEGATE(other))
		, m_K(*this, "K", NETLIB_DELEGATE(other))
		, m_clrQ(*this, "CLRQ", NETLIB_DELEGATE(other))
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_clk.set_state(logic_t::STATE_INP_HL);
			//m_Q.initial(0);
			//m_QQ.initial(1);
		}

		NETLIB_HANDLERI(other)
		{
			if (!m_clrQ())
			{
				m_clk.inactivate();
				newstate(0);
			}
			else if (m_J() | m_K())
				m_clk.activate_hl();
		}

		NETLIB_HANDLERI(clk)
		{
			const netlist_sig_t J(m_J());
			const netlist_sig_t K(m_K());
			const netlist_sig_t t(m_Q.net().Q());
			/*
			 *  J K  Q1 Q2 F t   Q
			 *  0 0   0  1 0 0   0
			 *  0 1   0  0 0 0   0
			 *  1 0   0  0 1 0   1
			 *  1 1   1  0 0 0   1
			 *  0 0   0  1 0 1   1
			 *  0 1   0  0 0 1   0
			 *  1 0   0  0 1 1   1
			 *  1 1   1  0 0 1   0
			 */
			if ((J & K) ^ 1)
				m_clk.inactivate();
			newstate(((t ^ 1) & J) | (t & (K ^ 1)));
		}

		void newstate(const netlist_sig_t state) noexcept
		{
			m_Q.push(state, D::delay::value(state));
			m_QQ.push(state ^ 1, D::delay::value(state ^ 1));
		}

		logic_input_t m_clk;

		logic_input_t m_J;
		logic_input_t m_K;
		logic_input_t m_clrQ;

		logic_output_t m_Q;
		logic_output_t m_QQ;

		nld_power_pins m_power_pins;
	};

	struct desc_74107 : public desc_base
	{
		using delay = times_ns2<16, 25>;
	};
	struct desc_74107A : public desc_base
	{
		using delay = times_ns2<15, 15>;
	};

	using NETLIB_NAME(74107) = NETLIB_NAME(74107_base)<desc_74107>;
	using NETLIB_NAME(74107A) = NETLIB_NAME(74107_base)<desc_74107A>;

	NETLIB_DEVICE_IMPL(74107,       "TTL_74107",    "+CLK,+J,+K,+CLRQ,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74107A,      "TTL_74107A",   "+CLK,+J,+K,+CLRQ,@VCC,@GND")

} // namespace netlist::devices
