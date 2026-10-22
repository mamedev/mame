// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Granblade infrared weapon for Webdiver Gladion. Reads Blade / Gran
    Magnum controls, remembers the last reported form and produces ten-bit
    code/complement reports through the shared infrared report generator.

    TODO:
    - Measure physical transmitter timing and repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_wdg_granblade.h"

DEFINE_DEVICE_TYPE(XAVIX_GRANBLADE, xavix_granblade_device, "xavix_granblade", "Webdiver Granblade Infrared Controller")

xavix_granblade_device::xavix_granblade_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_pulse_distance_device(mconfig, XAVIX_GRANBLADE, tag, owner, clock)
	, m_granblade_action(*this, "GRANBLADE_ACTION")
	, m_granblade_mode(*this, "GRANBLADE_MODE")
	, m_granblade_power(*this, "GRANBLADE_POWER")
{
	set_report_count(1);
	set_report_bits<0>(10);
	// Representative IR intervals are centred in the ROM's acceptance windows; no hardware capture is available.
	set_pulse_distance_ticks(0x28 * 256, 0x48 * 256, 0x2c * 256);
}

static INPUT_PORTS_START( xavix_granblade )
	PORT_START("GRANBLADE_ACTION")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Granblade Swing / Confirm") PORT_CODE(KEYCODE_Z) PORT_IMPULSE(1) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Gran Magnum Trigger / Fire") PORT_CODE(KEYCODE_X) PORT_IMPULSE(1) PORT_PLAYER(1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GRANBLADE_MODE")
	// The physical weapon retains its Blade or Gran Magnum form after transformation.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Transform Granblade (Blade / Gran Magnum)") PORT_CODE(KEYCODE_C) PORT_TOGGLE PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GRANBLADE_POWER")
	PORT_CONFNAME( 0x01, 0x01, "Granblade Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x01, DEF_STR(On) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_granblade_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_granblade);
}

void xavix_granblade_device::device_start()
{
	xavix_ir_pulse_distance_device::device_start();

	save_item(NAME(m_granblade_last_mode));
}

void xavix_granblade_device::device_reset()
{
	xavix_ir_pulse_distance_device::device_reset();

	m_granblade_last_mode = BIT(m_granblade_mode->read(), 0);
}

u32 xavix_granblade_device::read_report(unsigned)
{
	unsigned const mode = BIT(m_granblade_mode->read(), 0);
	bool const transform = mode != m_granblade_last_mode;
	m_granblade_last_mode = mode;
	// Report identities follow the ROM's paired mode/action handling and mode-specific tutorial prompts.
	u8 const code = GRANBLADE_REPORTS[mode][transform ? 0 : 1];
	return complemented_report(code);
}

bool xavix_granblade_device::report_enabled(unsigned)
{
	unsigned const mode = BIT(m_granblade_mode->read(), 0);
	return BIT(m_granblade_power->read(), 0)
		&& ((mode != m_granblade_last_mode) || BIT(m_granblade_action->read(), mode));
}
