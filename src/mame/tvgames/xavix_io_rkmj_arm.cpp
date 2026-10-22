// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Ryuusei no Rockman infrared arm controller. Encodes raised, neutral and
    lowered poses with the Shot gestures; a held Shake control alternates
    the reported pose. Power-off suppresses reports and clears that sequence.

    TODO:
    - Measure physical transmitter timing and repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_rkmj_arm.h"

DEFINE_DEVICE_TYPE(XAVIX_ROCKMAN_ARM, xavix_rockman_arm_device, "xavix_rockman_arm", "Rockman Infrared Arm Controller")

xavix_rockman_arm_device::xavix_rockman_arm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_framed_device(mconfig, XAVIX_ROCKMAN_ARM, tag, owner, clock)
	, m_arm(*this, "ARM")
	, m_controller_buttons(*this, "CONTROL")
	, m_controller_power(*this, "CONTROLLER_POWER")
	, m_factory_code(*this, "FACTORY_CODE")
{
	set_report_count(1);
	set_report_bits<0>(13);
	// Timings centre transitions in the firmware sample windows; physical transmitter timing and repetition remain unmeasured.
	set_start_ticks<0>(0x162 * 64);
	set_bit_ticks(0x89 * 64);
}

static INPUT_PORTS_START( xavix_rockman_arm )
	PORT_START("ARM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY PORT_NAME("Arm Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_NAME("Arm Down")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CONTROL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Shot / Confirm")
	// D02 is firmware-proven as Back, but its exact physical turned-button gesture is inferred from the single-button controller.
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Turn Controller and Press Shot / Back")
	// Holding this high-level control generates the alternating tilt phases scored as a shake.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Shake Arm")
	// Additional reports complete the factory checks; physical transmitter controls/fixture are unidentified.
	// PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Factory Test Transmission") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CONTROLLER_POWER")
	// The battery-powered arm controller has its own slide switch and no battery-level field in its report.
	PORT_CONFNAME( 0x01, 0x01, "Controller Power" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("FACTORY_CODE")
	// PORT_CONFNAME( 0x01, 0x00, "Factory Test Transmission Code" )
	// PORT_CONFSETTING(    0x00, "0D73" )
	// PORT_CONFSETTING(    0x01, "1645" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_rockman_arm_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_rockman_arm);
}

void xavix_rockman_arm_device::device_start()
{
	xavix_ir_framed_device::device_start();

	save_item(NAME(m_shake_frames));
	save_item(NAME(m_shake_lowered));
}

void xavix_rockman_arm_device::device_reset()
{
	xavix_ir_framed_device::device_reset();

	m_shake_frames = 0;
	m_shake_lowered = false;
}

bool xavix_rockman_arm_device::report_enabled(unsigned)
{
	if (BIT(m_controller_power->read(), 0))
		return 1;

	m_shake_frames = 0;
	m_shake_lowered = false;
	return 0;
}

u32 xavix_rockman_arm_device::read_report(unsigned)
{
	u8 const controls = m_controller_buttons->read();
	u8 const arm = m_arm->read() & 3;

	unsigned pose = arm == 1 ? 1 : arm == 2 ? 2 : 0;
	if (BIT(controls, 2))
	{
		pose = m_shake_lowered ? 2 : 1;
		if (++m_shake_frames == SHAKE_PHASE_FRAMES)
		{
			m_shake_frames = 0;
			m_shake_lowered = !m_shake_lowered;
		}
	}
	else
	{
		m_shake_frames = 0;
		m_shake_lowered = false;
	}

	if (BIT(controls, 3))
		return FACTORY_REPORTS[BIT(m_factory_code->read(), 0)];

	// Normal Shot takes priority if both high-level button gestures are mapped to one host control.
	unsigned const button = BIT(controls, 0) ? 1 : BIT(controls, 1) ? 2 : 0;
	return CONTROLLER_REPORTS[pose][button];
}
