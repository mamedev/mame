// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Comet-san Lovelin Baton. Reads the baton controls and TV/game mode,
    producing ten-bit code/complement reports through the shared infrared
    report generator and decoding the station's outgoing carrier bursts.

    TODO:
    - Measure physical transmitter timing and repetition.
    - Identify the station commands' light, sound and vibration effects; an IR return channel is inferred.

***************************************************************************/

#include "emu.h"
#include "xavix_io_comt_baton.h"

#define LOG_COMMAND (1U << 1)
#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_COMET_BATON, xavix_comet_baton_device, "xavix_comet_baton", "Comet-san Lovelin Baton Infrared Controller")

xavix_comet_baton_device::xavix_comet_baton_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_pulse_distance_device_base(mconfig, XAVIX_COMET_BATON, tag, owner, clock)
	, xavix_ir_receiver_interface(*this, 1'024, true)
	, m_baton(*this, "BATON")
	, m_baton_mode(*this, "BATON_MODE")
	, m_command(*this, "baton_command")
{
	set_report_count(1);
	set_report_bits<0>(10);
	// Representative intervals are centred within the firmware's measured short/long acceptance windows.
	set_pulse_distance_ticks(0x40 * 256, 0x60 * 256, 0x39 * 256);
}

static INPUT_PORTS_START( xavix_comet_baton )
	PORT_START("BATON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Baton Heart Button / Select") PORT_CODE(KEYCODE_Z) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Baton Shake / Confirm") PORT_CODE(KEYCODE_X) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Baton Big Shake") PORT_CODE(KEYCODE_C) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Baton Twirl") PORT_CODE(KEYCODE_V) PORT_IMPULSE(1)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BATON_MODE")
	PORT_CONFNAME( 0x01, 0x01, "Baton Switch" )
	PORT_CONFSETTING(    0x01, "Game" )
	PORT_CONFSETTING(    0x00, "TV" )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_comet_baton_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_comet_baton);
}

void xavix_comet_baton_device::device_start()
{
	xavix_ir_pulse_distance_device_base::device_start();

	save_item(NAME(m_received_bits));
	save_item(NAME(m_bit_count));
	save_item(NAME(m_receiving));
}

void xavix_comet_baton_device::device_reset()
{
	xavix_ir_pulse_distance_device_base::device_reset();
	ir_receiver_reset();
	m_received_bits = 0;
	m_bit_count = 0;
	m_receiving = false;
	m_command = 0xff;
}

u32 xavix_comet_baton_device::read_report(unsigned report)
{
	u8 const controls = m_baton->read();
	// The ROM also accepts combined Shake+Big Shake and Big Shake+Twirl reports.
	u8 const code = MOTION_REPORTS[BIT(controls, 1, 3)];
	return complemented_report(code);
}

bool xavix_comet_baton_device::reports_enabled()
{
	return BIT(m_baton_mode->read(), 0) && bool(m_baton->read() & 0x1f);
}

bool xavix_comet_baton_device::burst_length(u64 ticks, unsigned units)
{
	// Representative windows surround the native transmitter's roughly 7,400-clock units.
	return ticks >= 6'000 * units && ticks <= 9'500 * units;
}

void xavix_comet_baton_device::ir_burst_started(u64 gap_ticks)
{
	if (m_receiving)
	{
		if (m_bit_count < 10 && (burst_length(gap_ticks, 1) || burst_length(gap_ticks, 2)))
		{
			m_received_bits |= u16(burst_length(gap_ticks, 1)) << m_bit_count;
			m_bit_count++;
		}
		else
			m_receiving = false;
	}
}

void xavix_comet_baton_device::ir_idle_timeout()
{
	m_receiving = false;
}

void xavix_comet_baton_device::ir_burst_finished(u64 width_ticks)
{
	if (ir_receiver_level())
	{
		m_receiving = false;
		return;
	}

	if (burst_length(width_ticks, 3))
	{
		m_received_bits = 0;
		m_bit_count = 0;
		m_receiving = true;
	}
	else if (m_receiving && m_bit_count == 10)
	{
		// The two-unit footer terminates five LSB-first command bits and their complement.
		if (burst_length(width_ticks, 2) && (BIT(m_received_bits, 0, 5) ^ BIT(m_received_bits, 5, 5)) == 0x1f)
		{
			m_command = BIT(m_received_bits, 0, 5);
			LOGMASKED(LOG_COMMAND, "Station command %02x (unknown effect) at %s\n", BIT(m_received_bits, 0, 5), machine().time().as_string());
		}
		m_receiving = false;
	}
	else if (!burst_length(width_ticks, 1))
		m_receiving = false;

	if (m_receiving)
		ir_receiver_wait(28'500);
}
