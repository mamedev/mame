// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    XaviX Golf wireless swing sensor. Encodes optical coordinates and panel
    buttons into the two infrared report sections and generates a bounded
    swing trajectory from the high-level left/right-handed controls.

    TODO:
    - Verify physical transmitter timing and left-handed trajectories.

***************************************************************************/

#include "emu.h"
#include "xavix_io_xavgolf_swing.h"

DEFINE_DEVICE_TYPE(XAVIX_GOLF_SWING, xavix_golf_swing_device, "xavix_golf_swing", "XaviX Golf Swing Sensor")

xavix_golf_swing_device::xavix_golf_swing_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_framed_device(mconfig, XAVIX_GOLF_SWING, tag, owner, clock)
	, m_swing_active(*this, "SWING_ACTIVE")
	, m_swing_axis(*this, "SWING_%c", 'X')
	, m_swing_buttons(*this, "SWING_BUTTONS")
	, m_swing_power(*this, "SWING_POWER")
	, m_swing_phase(0)
{
	set_report_count(2);
	// The receiver's alternating timer phases sample each 0x80-cell twice.
	set_bit_ticks(0x80 * 64);
	set_start_ticks(0x80 * 64);
	set_report_bits<0>(11);
	set_report_bits<1>(9);
}

static INPUT_PORTS_START( xavix_golf_swing )
	PORT_START("SWING_ACTION")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Simulate Right-Handed Swing") PORT_CODE(KEYCODE_SPACE)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_golf_swing_device::swing_action), 0)
	// The optional left-handed set is documented, but reversing this trajectory is provisional without a sensor capture.
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Simulate Left-Handed Swing") PORT_CODE(KEYCODE_LALT)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_golf_swing_device::swing_action), 1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SWING_ACTIVE")
	// Keep a club in the sensor's field by default; hold V to remove it.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Remove Club (Raw)") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	// Raw controls expose the bounded optical coordinates for testing trajectories beyond the generated gestures.
	PORT_START("SWING_X")
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_X ) PORT_NAME("Swing Sensor X") PORT_CODE(MOUSECODE_X_INDEXED(0))
		PORT_MINMAX(0x01, 0x3e) PORT_SENSITIVITY(25) PORT_KEYDELTA(2) PORT_CENTERDELTA(0)
		PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_L) PORT_REVERSE

	PORT_START("SWING_Y")
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_Y ) PORT_NAME("Swing Sensor Y") PORT_CODE(MOUSECODE_Y_INDEXED(0))
		PORT_MINMAX(0x01, 0x3e) PORT_SENSITIVITY(25) PORT_KEYDELTA(2) PORT_CENTERDELTA(0)
		PORT_CODE_DEC(KEYCODE_I) PORT_CODE_INC(KEYCODE_K)

	PORT_START("SWING_BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Swing Sensor Enter") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Swing Sensor Cancel") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )   PORT_NAME("Swing Sensor Up")   PORT_CODE(KEYCODE_W) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Swing Sensor Down") PORT_CODE(KEYCODE_S) PORT_2WAY
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SWING_POWER")
	PORT_CONFNAME( 0x01, 0x01, "Swing Sensor Power" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

ioport_constructor xavix_golf_swing_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_golf_swing);
}

void xavix_golf_swing_device::device_start()
{
	xavix_ir_framed_device::device_start();

	save_item(NAME(m_swing_phase));
	save_item(NAME(m_swing_reverse));
}

void xavix_golf_swing_device::device_reset()
{
	xavix_ir_framed_device::device_reset();

	m_swing_phase = 0;
	m_swing_reverse = false;
}

INPUT_CHANGED_MEMBER(xavix_golf_swing_device::swing_action)
{
	if (newval && !oldval && !m_swing_phase && BIT(m_swing_power->read(), 0))
	{
		m_swing_phase = 1;
		m_swing_reverse = bool(param);
	}
}

bool xavix_golf_swing_device::reports_enabled()
{
	if (!BIT(m_swing_power->read(), 0))
	{
		m_swing_phase = 0;
		return 0;
	}

	return 1;
}

u32 xavix_golf_swing_device::read_report(unsigned report)
{
	bool const generated = m_swing_phase != 0;
	bool const sentinel = !generated && !BIT(m_swing_active->read(), 0);
	u8 const coord0 = generated ? (((m_swing_phase == 1) != m_swing_reverse) ? 0x01 : 0x3e)
		: sentinel ? 0x3f : m_swing_axis[0]->read() & 0x3f;
	u8 const coord1 = generated ? 0x20 : sentinel ? 0x3f : m_swing_axis[1]->read() & 0x3f;

	if (!report)
	{
		bool const bit2 = BIT(coord0, 1);
		bool const bit3 = BIT(coord1, 1);
		bool const bit4 = BIT(coord0, 4);
		bool const bit5 = BIT(coord1, 4);
		bool const bit6 = sentinel || !(bit4 || bit5);
		bool const bit7 = BIT(coord0, 0);
		bool const bit8 = BIT(coord1, 0);
		bool const bit9 = BIT(coord0, 3);
		bool const bit10 = BIT(coord1, 3);
		bool const bit11 = sentinel || !(bit9 || bit10);
		bool const bit12 = 1 ^ bit2 ^ bit3 ^ bit4 ^ bit5 ^ bit7 ^ bit8 ^ bit9 ^ bit10;
		u16 const logical = u16(bit2) | (u16(bit3) << 1) | (u16(bit4) << 2) | (u16(bit5) << 3) | (u16(bit6) << 4) |
			(u16(bit7) << 5) | (u16(bit8) << 6) | (u16(bit9) << 7) | (u16(bit10) << 8) | (u16(bit11) << 9) | (u16(bit12) << 10);
		return ~logical & 0x07ff;
	}

	bool const bit15 = BIT(coord0, 2);
	bool const bit16 = BIT(coord1, 2);
	bool const bit17 = BIT(coord0, 5);
	bool const bit18 = BIT(coord1, 5);
	bool const bit19 = sentinel || !(bit17 || bit18);
	u8 const buttons = m_swing_buttons->read();
	// The physical transmitter documents only one button at a time; simultaneous-button priority is unknown.
	u8 const symbol = BIT(buttons, 0) ? 0x60 : BIT(buttons, 1) ? 0x80 : BIT(buttons, 2) ? 0x40 : BIT(buttons, 3) ? 0xa0 : 0x00;
	bool const bit21 = BIT(symbol, 5);
	bool const bit22 = BIT(symbol, 6);
	bool const bit23 = BIT(symbol, 7);
	bool const bit20 = 1 ^ bit15 ^ bit16 ^ bit17 ^ bit18 ^ bit21 ^ bit22 ^ bit23;
	u16 const logical = u16(bit15) | (u16(bit16) << 1) | (u16(bit17) << 2) | (u16(bit18) << 3) | (u16(bit19) << 4) |
		(u16(bit20) << 5) | (u16(bit21) << 6) | (u16(bit22) << 7) | (u16(bit23) << 8);
	if (generated && ++m_swing_phase > 2)
		m_swing_phase = 0;
	return ~logical & 0x01ff;
}
