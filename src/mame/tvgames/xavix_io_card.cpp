// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Card-reader interfaces for Hello Kitty (epo_kabj) and Excite Stage
    (epo_es2j). Selected card numbers are encoded as timed optical scan events
    consumed by each system's reader interrupt handler. Scan intervals are
    representative values within the software's decoding windows.

    TODO:
    - Identify physical card-face names for the exposed barcode/card
      IDs.
    - Verify original optical scan timing.

***************************************************************************/

#include "emu.h"
#include "xavix_io_card.h"

#include <bit>

DEFINE_DEVICE_TYPE(XAVIX_EPO_KABJ_CARD_READER, xavix_epo_kabj_card_reader_device, "xavix_epo_kabj_card", "XaviX Hello Kitty Barcode Card Reader")
DEFINE_DEVICE_TYPE(XAVIX_EPO_ES2J_CARD_READER, xavix_epo_es2j_card_reader_device, "xavix_epo_es2j_card", "XaviX Excite Stage Optical Card Reader")

static INPUT_PORTS_START( xavix_epo_kabj_card_reader )
	// Physical card-face labels are not yet known, so expose the ROM's verified barcode number.
	PORT_START("CODE")
	PORT_ADJUSTER( 1, "Card Barcode" ) PORT_MINMAX(1, 172)

	PORT_START("SCAN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("Scan Card")
		PORT_WRITE_LINE_MEMBER(FUNC(xavix_card_reader_device::scan_card))
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_epo_es2j_card_reader )
	// Physical card-face names are not yet known, so expose the ROM's verified card ID.
	PORT_START("CARD")
	PORT_ADJUSTER( 0, "Card ID" ) PORT_MINMAX(0, 123)

	PORT_START("SCAN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Swipe Card")
		PORT_WRITE_LINE_MEMBER(FUNC(xavix_card_reader_device::scan_card))
INPUT_PORTS_END

xavix_card_reader_device::xavix_card_reader_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_event_out_cb(*this)
	, m_scanning(false)
{
}

xavix_epo_kabj_card_reader_device::xavix_epo_kabj_card_reader_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock)
	: xavix_card_reader_device(mconfig, XAVIX_EPO_KABJ_CARD_READER, tag, owner, clock)
	, m_code(*this, "CODE")
	, m_pattern(0)
{
}

xavix_epo_es2j_card_reader_device::xavix_epo_es2j_card_reader_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock)
	: xavix_card_reader_device(mconfig, XAVIX_EPO_ES2J_CARD_READER, tag, owner, clock)
	, m_card(*this, "CARD")
	, m_pattern(0)
{
}

ioport_constructor xavix_epo_kabj_card_reader_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( xavix_epo_kabj_card_reader );
}

ioport_constructor xavix_epo_es2j_card_reader_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( xavix_epo_es2j_card_reader );
}

void xavix_card_reader_device::device_start()
{
	m_event_out_cb.resolve_safe();

	m_event_timer = timer_alloc(FUNC(xavix_card_reader_device::event_timer), this);

	save_item(NAME(m_event_count));
	save_item(NAME(m_event_index));
	save_item(NAME(m_scanning));
}

void xavix_card_reader_device::device_reset()
{
	m_event_timer->adjust(attotime::never);
	m_event_count = 0;
	m_event_index = 0;
	m_scanning = false;
}

void xavix_epo_kabj_card_reader_device::device_start()
{
	xavix_card_reader_device::device_start();
	save_item(NAME(m_pattern));
}

void xavix_epo_es2j_card_reader_device::device_start()
{
	assert(clock());
	xavix_card_reader_device::device_start();
	save_item(NAME(m_pattern));
}

void xavix_card_reader_device::scan_card(int state)
{
	if (!state || m_scanning)
		return;

	m_event_count = begin_scan();
	if (!m_event_count)
		return;

	m_event_index = 0;
	m_scanning = true;
	m_event_timer->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(xavix_card_reader_device::event_timer)
{
	m_event_out_cb();
	if (++m_event_index == m_event_count)
	{
		m_event_timer->adjust(attotime::never);
		m_scanning = false;
	}
	else
	{
		m_event_timer->adjust(interval_after_event(m_event_index));
	}
}

u8 xavix_epo_kabj_card_reader_device::begin_scan()
{
	u8 const code = m_code->read();
	u8 const checksum = (8 - std::popcount(code)) & 7;
	m_pattern = u16(checksum) << 10 | 0x2000;
	for (unsigned bit = 0; bit != 8; ++bit)
		m_pattern |= u16(BIT(code, 7 - bit)) << (bit + 2);

	// The reader produces one extra trailing interval for codes with at most two set bits.
	return std::popcount(code) <= 2 ? 17 : 16;
}

attotime xavix_epo_kabj_card_reader_device::interval_after_event(u8 events_emitted) const
{
	// Millisecond-scale 2:1 periods survive interrupt contention while remaining below the reader timeout.
	return attotime::from_msec(BIT(m_pattern, events_emitted - 1) ? 4 : 2);
}

u8 xavix_epo_es2j_card_reader_device::begin_scan()
{
	u32 const stream = CARD_STREAMS[m_card->read()];
	m_pattern = stream & 0x00ffffff;
	return 23 + ((stream >> 24) & 0x03);
}

attotime xavix_epo_es2j_card_reader_device::interval_after_event(u8 events_emitted) const
{
	if (events_emitted == 1)
		return attotime::from_msec(16);
	if (events_emitted == 2)
		return attotime::from_ticks(0x1800 * 34, clock());

	// Allow the interrupt-driven width counter to run between representative 2:1 stripes.
	u8 const bit = events_emitted - 1;
	u32 const counts = bit < 24 && BIT(m_pattern, bit) ? 0x2000 : 0x1000;
	return attotime::from_ticks(counts * 34, clock());
}
