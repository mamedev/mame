// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Signetics PLS105 (aka 82S105) Field-Programmable Logic Sequencer
 *
 *              ___ ___
 *     CLK   1 |   U   | 28  Vcc
 *      I7   2 |       | 27  I8
 *      I6   3 |       | 26  I9
 *      I5   4 |       | 25  I10
 *      I4   5 |       | 24  I11
 *      I3   6 |       | 23  I12
 *      I2   7 |       | 22  I13
 *      I1   8 |       | 21  I14
 *      I0   9 |       | 20  I15
 *      F7  10 |       | 19  PR/OE*
 *      F6  11 |       | 18  F0
 *      F5  12 |       | 17  F1
 *      F4  13 |       | 16  F2
 *     GND  14 |_______| 15  F3
 *
 * Sources:
 *  - PLD Data Manual 1987, Signetics Corporation
 *  - Logic Diagram Package, 981-0251, September 1990, Data I/O Corporation
 *
 * TODO:
 *  - diagnostic function
 */

#include "emu.h"
#include "pls105.h"

#include "multibyte.h"

#define LOG_IO    (1U << 1)
#define LOG_STATE (1U << 2)

//#define VERBOSE (LOG_GENERAL|LOG_IO|LOG_STATE)
#include "logmacro.h"

static constexpr unsigned FUSE_TERMS = 48;
static constexpr unsigned FUSE_ROWS = 74;
static constexpr unsigned FUSE_COUNT = FUSE_TERMS * FUSE_ROWS + 1;
static constexpr unsigned FUSE_BYTES = 4 + (FUSE_COUNT + 7) / 8;

DEFINE_DEVICE_TYPE(PLS105, pls105_device, "pls105", "Signetics Field-Programmable Logic Sequencer (16x48x8)")

pls105_device::pls105_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PLS105, tag, owner, clock)
	, m_fuse(*this, DEVICE_SELF)
	, m_f_cb(*this)
	, m_pr(true)
	, m_i(0xffff)
	, m_p(0x3f)
	, m_f(0xff)
	, m_fuse_i{}
	, m_fuse_p{}
	, m_fuse_c{}
	, m_fuse_n{}
	, m_fuse_f{}
{
}

void pls105_device::device_start()
{
	save_item(NAME(m_pr));
	save_item(NAME(m_i));
	save_item(NAME(m_p));
	save_item(NAME(m_f));

	decode();

	m_update = timer_alloc(FUNC(pls105_device::update), this);

	if (!m_option)
		output();
}

void pls105_device::device_reset()
{
}

void pls105_device::pr_w(int state)
{
	if (bool(state) != m_pr)
	{
		if (!m_option)
		{
			if (state)
			{
				LOG("preset\n");
				m_p = 0x3f;
				m_f = 0xff;

				output();
			}
			else
				m_update->adjust(next_cycle());
		}
		else if (m_option && !state)
		{
			LOG("output enabled\n");
			output();
		}

		m_pr = bool(state);
	}
}

template <unsigned N> void pls105_device::i_w(int state)
{
	if (BIT(m_i, N) != state)
	{
		LOGMASKED(LOG_IO, "I%u %d\n", N, state);

		if (state)
			m_i |= 1U << N;
		else
			m_i &= ~(1U << N);

		if (!m_update->enabled())
			m_update->adjust(next_cycle());
	}
}

template void pls105_device::i_w<0U>(int state);
template void pls105_device::i_w<1U>(int state);
template void pls105_device::i_w<2U>(int state);
template void pls105_device::i_w<3U>(int state);
template void pls105_device::i_w<4U>(int state);
template void pls105_device::i_w<5U>(int state);
template void pls105_device::i_w<6U>(int state);
template void pls105_device::i_w<7U>(int state);
template void pls105_device::i_w<8U>(int state);
template void pls105_device::i_w<9U>(int state);
template void pls105_device::i_w<10U>(int state);
template void pls105_device::i_w<11U>(int state);
template void pls105_device::i_w<12U>(int state);
template void pls105_device::i_w<13U>(int state);
template void pls105_device::i_w<14U>(int state);
template void pls105_device::i_w<15U>(int state);

/*
 * From the device programmer perspective, fuses are ordered by term (I×16,
 * P×6, C, N×6, F×8), with two rows each by 48 terms giving 3552 logic term
 * fuses, followed by a final option fuse. This function transforms the fuse
 * data from the programmer order into row-oriented arrays, using 48 bits to
 * represent the terms in parallel enabling efficient emulation. A value of 1
 * in the fuse data indicates that the fuse is blown.
 */
void pls105_device::decode()
{
	// binary fuse data format starts with big-endian 32-bit fuse count, then
	// individual fuses packed into bytes, LSB first (0=intact, 1=blown).

	if (m_fuse.bytes() != FUSE_BYTES)
		throw emu_fatalerror("%s: fuse bytes %u must be exactly %u\n", tag(), m_fuse.bytes(), FUSE_BYTES);

	unsigned const count = get_u32be(m_fuse);
	if (count != FUSE_COUNT)
		throw emu_fatalerror("%s: fuse count %u must be exactly %u\n", tag(), count, FUSE_COUNT);

	for (unsigned term = 0; term < FUSE_TERMS; term++)
	{
		u64 const t = 1ULL << term;

		for (unsigned row = 0; row < FUSE_ROWS; row++)
		{
			unsigned const fuse = term * FUSE_ROWS + row;

			if (BIT(m_fuse[(fuse >> 3) + 4], fuse & 7))
			{
				if (row < 32)
					m_fuse_i[row & 1][row >> 1] |= t;
				else if (row < 44)
					m_fuse_p[row & 1][(row - 32) >> 1] |= t;
				else if (row < 46)
					m_fuse_c[row & 1] |= t;
				else if (row < 58)
					m_fuse_n[row & 1][(row - 46) >> 1] |= t;
				else if (row < 74)
					m_fuse_f[row & 1][(row - 58) >> 1] |= t;
			}
		}
	}

	unsigned const fuse = FUSE_COUNT - 1;
	m_option = BIT(m_fuse[(fuse >> 3) + 4], fuse & 7);
}

attotime pls105_device::next_cycle() const
{
	attotime const time = machine().time();

	return attotime::from_ticks(time.as_ticks(clock()) + 1, clock()) - time;
}

void pls105_device::update(s32 param)
{
	LOGMASKED(LOG_STATE, "update P %u (%s)\n", m_p, machine().time().as_string());

	// clocking disabled by preset
	if (!m_option && m_pr)
		return;

	u8 const p = m_p;
	u8 const f = m_f;

	u64 data = util::make_bitmask<u64>(FUSE_TERMS);

	/*
	 * AND array emulation uses inverted logic (i.e., select the True term when
	 * the input is 0, and the Complement when the input is 1). Combined with
	 * the device fuse polarity (where 0=intact), this avoids several inversion
	 * operations and produces correct results for the INACTIVE and DON'T CARE
	 * cases without additional logic.
	 */

	// input
	for (unsigned i = 0; i < std::size(*m_fuse_i); i++)
		data &= m_fuse_i[BIT(m_i, i)][i];

	// present state
	for (unsigned i = 0; i < std::size(*m_fuse_p); i++)
		data &= m_fuse_p[BIT(m_p, i)][i];

	// complement
	if (data & ~m_fuse_c[1])
		data &= m_fuse_c[0];

	if ((VERBOSE & LOG_STATE) && data)
	{
		std::string terms;

		for (unsigned i = 0; i < FUSE_TERMS; i++)
		{
			if (BIT(data, i))
			{
				if (!terms.empty())
					terms.append(",");

				terms.append(util::string_format("%02u", i));
			}
		}

		LOGMASKED(LOG_STATE, "active terms %s\n", terms);
	}

	// next state
	for (unsigned i = 0; i < std::size(*m_fuse_n); i++)
	{
		bool const s = data & ~m_fuse_n[0][i];
		bool const r = data & ~m_fuse_n[1][i];

		if (s && !r)
			m_p |= 1U << i;
		else if (r && !s)
			m_p &= ~(1U << i);
	}

	// next output
	for (unsigned i = 0; i < std::size(*m_fuse_f); i++)
	{
		bool const s = data & ~m_fuse_f[0][i];
		bool const r = data & ~m_fuse_f[1][i];

		if (s && !r)
			m_f |= 1U << i;
		else if (r && !s)
			m_f &= ~(1U << i);
	}

	// output
	if (u8 const delta = f ^ m_f)
	{
		if (!m_option || !m_pr)
			output(delta);
	}

	// buried state change
	if (p ^ m_p)
	{
		LOGMASKED(LOG_STATE, "N %u\n", m_p);
		m_update->adjust(next_cycle());
	}
}

void pls105_device::output(u8 mask)
{
	for (unsigned i = 0; i < 8; i++)
	{
		if (BIT(mask, i))
		{
			LOGMASKED(LOG_IO, "F%u %d\n", i, BIT(m_f, i));
			m_f_cb[i](BIT(m_f, i));
		}
	}
}
