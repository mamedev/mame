// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Beyblade Ultimate ripcord encoder interface. Converts changes in host
    ripcord position into a train of I/O events spread across each video field,
    allowing the console software to estimate the launcher pull rate.

    TODO:
    - Measure physical encoder counts per pull distance and event
      timing.

***************************************************************************/

#include "emu.h"
#include "xavix_io_beyu_ripcord.h"

DEFINE_DEVICE_TYPE(XAVIX_BEYBLADE_RIPCORD, xavix_beyblade_ripcord_device, "xavix_beyblade_ripcord", "XaviX Beyblade Ripcord")

xavix_beyblade_ripcord_device::xavix_beyblade_ripcord_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_BEYBLADE_RIPCORD, tag, owner, clock)
	, m_event_out_cb(*this)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_position(*this, "RIPCORD")
{
}

static INPUT_PORTS_START( xavix_beyblade_ripcord )
	PORT_START("RIPCORD")
	// Patent JP3532900B2 identifies a rack-driven flywheel photointerrupter; host scaling is provisional.
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_NAME("Ripcord") PORT_SENSITIVITY(100) PORT_KEYDELTA(12) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(KEYCODE_SPACE) PORT_CODE(MOUSECODE_Y_INDEXED(0))
INPUT_PORTS_END

ioport_constructor xavix_beyblade_ripcord_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_beyblade_ripcord);
}

void xavix_beyblade_ripcord_device::device_start()
{
	m_event_out_cb.resolve_safe();

	m_event_timer = timer_alloc(FUNC(xavix_beyblade_ripcord_device::event_timer), this);

	save_item(NAME(m_last_position));
	save_item(NAME(m_events_remaining));
	save_item(NAME(m_position_valid));
}

void xavix_beyblade_ripcord_device::device_reset()
{
	m_event_timer->adjust(attotime::never);
	m_last_position = 0;
	m_events_remaining = 0;
	m_position_valid = false;
}

void xavix_beyblade_ripcord_device::vblank_w(int state)
{
	if (!state)
		return;

	u8 const position = m_position->read();
	if (!m_position_valid)
	{
		m_last_position = position;
		m_position_valid = true;
		return;
	}

	int const delta = s8(position - m_last_position);
	m_last_position = position;
	m_events_remaining = delta < 0 ? -delta : delta;
	if (!m_events_remaining)
		return;

	// Spread the encoder events across the field so firmware measures the pull rate rather than one burst.
	attotime const interval = m_screen->frame_period() / (m_events_remaining + 1);
	m_event_timer->adjust(interval, 0, interval);
}

TIMER_CALLBACK_MEMBER(xavix_beyblade_ripcord_device::event_timer)
{
	m_event_out_cb();
	if (!--m_events_remaining)
		m_event_timer->adjust(attotime::never);
}
