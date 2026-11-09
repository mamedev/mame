// license: BSD-3-Clause
// copyright-holders: Peter Ferrie, Angelo Salese
/**************************************************************************************************

Aladdin HASP dongle for Savage Quest

Converted from legacy implementation in misc/savquest.cpp
Passmode 1 product request: decoded from pin transitions. After the 0x9c, 0x9e
prefix the host sends 13 address bits, one per data bit 5 pulse, with data bit 6
sampled on the rising edge; the request completes on the falling edge of the 13th
pulse. Passmode 3 advancement: falling edge (0xbe -> 0x9e) of data bit 5.

**************************************************************************************************/

#include "emu.h"
#include "hasp_savquest.h"

#include <algorithm>
#include <cstring>

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(HASP_SAVQUEST, hasp_savquest_device, "hasp_savquest", "Aladdin HASP dongle for Savage Quest")

namespace {

constexpr uint8_t s_hasp_cmppass[] = {
	0xc3, 0xd9, 0xd3, 0xfb, 0x9d, 0x89, 0xb9, 0xa1, 0xb3, 0xc1, 0xf1, 0xcd, 0xdf, 0x9d
};

constexpr uint8_t s_hasp_prodinfo[] = {
	0x51, 0x4c, 0x52, 0x4d, 0x53, 0x4e, 0x53, 0x4e, 0x53, 0x49, 0x53, 0x48, 0x53, 0x4b, 0x53, 0x4a,
	0x53, 0x43, 0x53, 0x45, 0x52, 0x46, 0x53, 0x43, 0x53, 0x41, 0xac, 0x40, 0x53, 0xbc, 0x53, 0x42,
	0x53, 0x57, 0x53, 0x5d, 0x52, 0x5e, 0x53, 0x5b, 0x53, 0x59, 0xac, 0x58, 0x53, 0xa4
};

} // anonymous namespace

hasp_savquest_device::hasp_savquest_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HASP_SAVQUEST, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_data_in(0),
	m_prev_data(0),
	m_deferred_pending(false),
	m_deferred_token(0),
	m_select_in_state(1),
	m_haspind(0),
	m_haspstate(HASPSTATE_NONE),
	m_hasp_passind(0),
	m_hasp_passmode(0),
	m_hasp_prodind(0),
	m_port379_bit5(0),
	m_req_stage(0),
	m_req_prefix_ok(false),
	m_req_bits(0),
	m_req_acc(0)
{
	std::fill(std::begin(m_hasp_tmppass), std::end(m_hasp_tmppass), 0);
}

void hasp_savquest_device::device_start()
{
	save_item(NAME(m_data_in));
	save_item(NAME(m_prev_data));
	save_item(NAME(m_deferred_pending));
	save_item(NAME(m_deferred_token));
	save_item(NAME(m_select_in_state));
	save_item(NAME(m_haspind));
	save_item(NAME(m_haspstate));
	save_item(NAME(m_hasp_passind));
	save_item(NAME(m_hasp_tmppass));
	save_item(NAME(m_hasp_passmode));
	save_item(NAME(m_hasp_prodind));
	save_item(NAME(m_port379_bit5));
	save_item(NAME(m_req_stage));
	save_item(NAME(m_req_prefix_ok));
	save_item(NAME(m_req_bits));
	save_item(NAME(m_req_acc));
}

void hasp_savquest_device::device_reset()
{
	// Retain electrical pin state: m_data_in and m_select_in_state reflect physical lines
	m_prev_data = m_data_in;
	m_deferred_pending = false;
	++m_deferred_token;
	m_haspind = 0;
	m_haspstate = HASPSTATE_NONE;
	m_hasp_passind = 0;
	m_hasp_passmode = 0;
	m_hasp_prodind = 0;
	m_port379_bit5 = 0;
	m_req_stage = 0;
	m_req_prefix_ok = false;
	m_req_bits = 0;
	m_req_acc = 0;
	std::fill(std::begin(m_hasp_tmppass), std::end(m_hasp_tmppass), 0);

	output_fault(0);
	output_select(0);
	output_ack(0);
	output_busy(1);
	output_perror(m_port379_bit5);
}

void hasp_savquest_device::input_select_in(int state)
{
	m_select_in_state = state;
	LOG("input_select_in %d\n", state);
}

void hasp_savquest_device::update_bit(int bit, int state)
{
	if (state)
		m_data_in |= (1 << bit);
	else
		m_data_in &= ~(1 << bit);

	if (!m_deferred_pending)
	{
		m_deferred_pending = true;
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(hasp_savquest_device::deferred_update), this), int(++m_deferred_token));
	}
}

TIMER_CALLBACK_MEMBER(hasp_savquest_device::deferred_update)
{
	if (!m_deferred_pending || (param != int(m_deferred_token)))
		return;
	m_deferred_pending = false;
	process_byte(m_data_in);
}

uint8_t hasp_savquest_device::get_prodinfo_bit(int prodind) const
{
	// Index 0 returns 0, as the legacy code did in the build it was tested with: it read
	// m_hasp_prodinfo[-1] here, one byte before the table, and that read returned 0.
	if (prodind == 0)
		return 0;

	if ((prodind > 0) && (prodind <= int(sizeof(s_hasp_prodinfo) * 8)))
		return (s_hasp_prodinfo[(prodind - 1) >> 3] >> ((8 - prodind) & 7)) & 1;

	// Past the table, and for negative indices (the legacy int/size_t comparison sent
	// those here too), alternate between the two key values.
	return ((0x534d ^ ((prodind - 1) >> 4)) >> ((16 - prodind) & 15)) & 1;
}

void hasp_savquest_device::request_transition(uint8_t prev, uint8_t data8)
{
	// Called only for a pin change (Centronics does not report rewrites of the same value).
	if (m_req_stage == 0)
	{
		m_req_prefix_ok = (data8 == 0x9c);
		m_req_stage = 1;
		return;
	}
	if (m_req_stage == 1)
	{
		m_req_prefix_ok = m_req_prefix_ok && (data8 == 0x9e);
		m_req_stage = 2;
		return;
	}

	bool const rising = !BIT(prev, 5) && BIT(data8, 5);
	bool const falling = BIT(prev, 5) && !BIT(data8, 5);

	if (rising && (m_req_bits < 13))
	{
		// data bit 6 is held across the clock pulse; sample it on the rising edge
		m_req_acc = (m_req_acc << 1) | BIT(data8, 6);
		++m_req_bits;
		return;
	}

	if (!falling || (m_req_bits < 13))
		return;

	// falling edge of the 13th pulse completes the request
	// Multiply rather than shift, because the difference can be negative: an accumulator
	// below 0xc08 (0xc00 gives -128) yields a negative start index, which
	// get_prodinfo_bit() answers with the default key alternation.
	int const start = (m_req_acc - 0xc08) * 16;
	bool const cutoff_passed = start < (0x38 << 4);
	m_hasp_prodind = start;
	if (m_req_prefix_ok && cutoff_passed)
	{
		// The host samples the first response bit right after this low-phase write;
		// each later falling edge presents the next bit.
		m_hasp_passmode = 3;
		m_port379_bit5 = get_prodinfo_bit(start);
		m_hasp_prodind = start + 1;
	}
	m_haspstate = HASPSTATE_READ;
	LOG("hasp product query: acc=%03x start=%d prefix_ok=%d cutoff_passed=%d passmode=%d presented_idx=%d bit5=%d\n",
		m_req_acc, start, m_req_prefix_ok ? 1 : 0, cutoff_passed ? 1 : 0, m_hasp_passmode, start, m_port379_bit5);
}

void hasp_savquest_device::process_byte(uint8_t data8)
{
	if (m_select_in_state)
		return;

	output_fault(0);
	output_select(0);
	output_ack(0);
	output_busy(1);

	LOG("hasp write byte: %02x (state=%d passmode=%d ind=%d prev=%02x)\n", data8, m_haspstate, m_hasp_passmode, m_haspind, m_prev_data);

	/* state machine to determine when password is about to be entered */
	switch (m_haspind)
	{
	case 0:
		if (data8 == 0xc6)
		{
			++m_haspind;
			break;
		}
		m_haspind = 0;
		break;

	case 1:
		if (data8 == 0xc7)
		{
			++m_haspind;
			break;
		}
		m_haspind = 0;
		break;

	case 2:
		if (data8 == 0xc6)
		{
			++m_haspind;
			break;
		}
		m_haspind = 0;
		m_haspstate = HASPSTATE_NONE;
		break;

	case 3:
		m_haspind = 0;
		if (data8 == 0x80)
		{
			m_haspstate = HASPSTATE_PASSBEG;
			m_hasp_passind = 0;
			m_port379_bit5 = 0;
			m_prev_data = data8;
			output_perror(m_port379_bit5);
			LOG("hasp state -> PASSBEG, status reply bit5=%d\n", m_port379_bit5);
			return;
		}
		break;

	default:
		break;
	}

	if (!(m_haspstate == HASPSTATE_READ && m_hasp_passmode == 3))
		m_port379_bit5 = 0;

	if (m_haspstate == HASPSTATE_READ)
	{
		/* different passwords cause different values to be returned
		   but there are really only two passwords of interest
		   passmode 2 is used to verify that the dongle is responding correctly
		*/
		if (m_hasp_passmode == 2)
		{
			/* in passmode 2, some values remain unknown: 96, 9a, c4, d4, ec, f8
			   they all return 00, but if that's wrong then there will be failures to start
			*/
			if ((data8 == 0x94)
				|| (data8 == 0x9e)
				|| (data8 == 0xa4)
				|| (data8 == 0xb2)
				|| (data8 == 0xbe)
				|| (data8 == 0xd0))
			{
				m_prev_data = data8;
				output_perror(m_port379_bit5);
				LOG("hasp passmode 2 (neg) reply bit5=%d\n", m_port379_bit5);
				return;
			}

			if ((data8 == 0x8a)
				|| (data8 == 0x8e)
				|| (data8 == 0xca)
				|| (data8 == 0xd2)
				|| (data8 == 0xe2)
				|| (data8 == 0xf0)
				|| (data8 == 0xfc))
			{
				m_port379_bit5 = 1;
				m_prev_data = data8;
				output_perror(m_port379_bit5);
				LOG("hasp passmode 2 (pos) reply bit5=%d\n", m_port379_bit5);
				return;
			}
		}
		else if (m_hasp_passmode == 3)
		{
			bool const clock_edge = (BIT(m_prev_data, 5) == 1) && (BIT(data8, 5) == 0);
			if (clock_edge)
			{
				m_port379_bit5 = get_prodinfo_bit(m_hasp_prodind);
				LOG("hasp passmode 3 present idx=%d bit5=%d\n", m_hasp_prodind, m_port379_bit5);
				++m_hasp_prodind;
			}
			// Hold output unchanged on inactive clock phase

			m_prev_data = data8;
			output_perror(m_port379_bit5);
			LOG("hasp passmode 3 (prodinfo) reply bit5=%d (prodind=%d edge=%d)\n", m_port379_bit5, m_hasp_prodind, clock_edge ? 1 : 0);
			return;
		}

		/* in passmode 0, some values remain unknown: 8a, 8e (inconclusive), 94, 96, 9a, a4, b2, be, c4, d2, d4 (inconclusive), e2, ec, f8, fc
		   this is less of a concern since the contents seem to decrypt correctly
		*/
		static constexpr uint8_t s_passmode0_bit5[] = {
			0x88, 0x94, 0x98, 0x9c, 0x9e, 0xa0, 0xa4, 0xaa, 0xae, 0xb0, 0xb2, 0xbc, 0xbe, 0xc2, 0xc6, 0xc8,
			0xce, 0xd0, 0xd6, 0xd8, 0xdc, 0xe0, 0xe6, 0xea, 0xee, 0xf2, 0xf6
		};
		if (std::find(std::begin(s_passmode0_bit5), std::end(s_passmode0_bit5), data8) != std::end(s_passmode0_bit5))
			m_port379_bit5 = 1;
	}
	else if (m_haspstate == HASPSTATE_PASSEND)
	{
		if (data8 & 1)
		{
			if ((m_hasp_passmode == 1)
				&& (data8 == 0x9d))
			{
				m_hasp_passmode = 2;
			}

			m_haspstate = HASPSTATE_READ;
			LOG("hasp state -> READ (passmode=%d)\n", m_hasp_passmode);
		}
		else if (m_hasp_passmode == 1)
		{
			request_transition(m_prev_data, data8);
		}
	}
	else if ((m_haspstate == HASPSTATE_PASSBEG)
		&& (data8 & 1))
	{
		m_hasp_tmppass[m_hasp_passind] = data8;

		if (++m_hasp_passind == sizeof(s_hasp_cmppass))
		{
			m_haspstate = HASPSTATE_PASSEND;
			m_hasp_passind = 0;
			m_hasp_passmode = (int) !memcmp(m_hasp_tmppass, s_hasp_cmppass, sizeof(s_hasp_cmppass));
			m_req_stage = 0;
			m_req_prefix_ok = false;
			m_req_bits = 0;
			m_req_acc = 0;
			LOG("hasp state -> PASSEND (passmode=%d)\n", m_hasp_passmode);
		}
	}

	m_prev_data = data8;
	output_perror(m_port379_bit5);
	LOG("hasp status reply: bit5=%d\n", m_port379_bit5);
}
