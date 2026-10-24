// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    xavbassf wireless tackle. Its nRF24E1 runs an external EEPROM program,
    reading buttons, a rod sensor and a reel encoder. The integrated radio
    exchanges input reports and motor commands with the cartridge.

    TODO:
    - Measure the rod sensor's voltage range and reel encoder pitch.
    - Verify the physical reel winding direction.

***************************************************************************/

#include "emu.h"
#include "xavix_io_xavbassf_tackle.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(XAVIX_BASSFISHING, xavix_bassfishing_device, "xavix_bassfishing", "XaviX Bass Fishing Wireless Tackle")

xavix_bassfishing_device::xavix_bassfishing_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_BASSFISHING, tag, owner, clock)
	, m_mcu(*this, "mcu")
	, m_eeprom(*this, "eeprom")
	, m_controls(*this, "TACKLE_BUTTONS")
	, m_rod(*this, "ROD")
	, m_reel(*this, "REEL")
	, m_power(*this, "TACKLE_POWER")
	, m_reaction_battery(*this, "REACTION_BATTERIES")
	, m_motor_drive(*this, "reaction_motor_drive")
	, m_reaction(*this, "reaction")
{
}

static INPUT_PORTS_START( xavix_bassfishing )
	PORT_START("TACKLE_BUTTONS")
	// The radio MCU folds analogue Enter/Cancel switches and digital Select contacts into this order.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Fishing Tackle Enter") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Fishing Tackle Cancel") PORT_CODE(KEYCODE_X)
	// The firmware enters standby when Up and Down are pressed together.
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_NAME("Fishing Tackle Select Down")  PORT_CODE(KEYCODE_S) PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_NAME("Fishing Tackle Select Left")  PORT_CODE(KEYCODE_A) PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Fishing Tackle Select Right") PORT_CODE(KEYCODE_D) PORT_16WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_NAME("Fishing Tackle Select Up")    PORT_CODE(KEYCODE_W) PORT_16WAY

	PORT_START("ROD")
	PORT_BIT( 0x0f, 0x08, IPT_AD_STICK_Y ) PORT_NAME("Fishing Rod Motion") PORT_MINMAX(0, 15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_CODE_DEC(KEYCODE_Q) PORT_CODE_INC(KEYCODE_E) PORT_CODE(MOUSECODE_Y_INDEXED(0))

	PORT_START("REEL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Reel Handle") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_T) PORT_CODE_INC(KEYCODE_Y) PORT_CODE(MOUSECODE_X_INDEXED(0))

	PORT_START("TACKLE_POWER")
	PORT_CONFNAME( 0x01, 0x01, "Fishing Tackle Power" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

	PORT_START("REACTION_BATTERIES")
	PORT_CONFNAME( 0x01, 0x01, "Reaction Batteries" )
	PORT_CONFSETTING(    0x00, "Not Installed" )
	PORT_CONFSETTING(    0x01, "Installed" )
INPUT_PORTS_END

ioport_constructor xavix_bassfishing_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_bassfishing);
}

void xavix_bassfishing_device::device_add_mconfig(machine_config &config)
{
	// TODO: Identify the fitted EEPROM; the dump and SPI protocol are 25320-compatible.
	EEPROM_25320(config, m_eeprom);

	NRF24E1(config, m_mcu, 16_MHz_XTAL);
	m_mcu->set_eeprom_region("^eeprom");
	m_mcu->eeprom_read_cb().set([this] (offs_t offset) -> u8 { return m_eeprom->internal_read(offset); });
	m_mcu->port_in_cb<0>().set(FUNC(xavix_bassfishing_device::tackle_port_r));
	m_mcu->port_in_cb<1>().set([this] () -> u8 { return 0xfc | m_reel_phase; });
	m_mcu->gpio_out_cb<0>().set(FUNC(xavix_bassfishing_device::tackle_port_w));
	m_mcu->adc_read_cb().set(FUNC(xavix_bassfishing_device::tackle_adc_r));
	m_mcu->spi_clock_cb().set(m_eeprom, FUNC(eeprom_spi_base_device::sck_w));
	m_mcu->spi_mosi_cb().set(m_eeprom, FUNC(eeprom_spi_base_device::si_w));
	m_mcu->spi_miso_cb().set(m_eeprom, FUNC(eeprom_spi_base_device::so_r));
}

void xavix_bassfishing_device::device_start()
{
	m_input_timer = timer_alloc(FUNC(xavix_bassfishing_device::sample_inputs), this);
	save_item(NAME(m_last_reel));
	save_item(NAME(m_reel_phase));
	save_item(NAME(m_reel_pending));
	save_item(NAME(m_powered));
}

void xavix_bassfishing_device::device_reset()
{
	m_last_reel = m_reel->read();
	m_reel_phase = 3;
	m_reel_pending = 0;
	m_motor_drive = 0;
	m_powered = bool(m_power->read());
	m_reaction = 0;
	m_input_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
}

void xavix_bassfishing_device::device_reset_after_children()
{
	m_mcu->power_w(m_powered);
}

u8 xavix_bassfishing_device::tackle_port_r()
{
	u8 const buttons = m_controls->read();
	// Enter also reaches the digital wake-up pin while the ADC is asleep.
	return (0xff ^ (buttons & 0xf0)) & ~(BIT(buttons, 2) << 3);
}

u16 xavix_bassfishing_device::tackle_adc_r(offs_t channel)
{
	switch (channel)
	{
	case 0:
		// Representative sensor range; the firmware learns its resting voltage.
		return (0x70 + 2 * (m_rod->read() & 0x0f)) << 4;
	case 1: return BIT(m_controls->read(), 2) ? 0 : 0x0fff;
	case 2: return BIT(m_controls->read(), 3) ? 0 : 0x0fff;
	case 8: return 0x0d1c; // Nominal 3 V supply measured against the internal reference.
	default: return 0x0fff;
	}
}

void xavix_bassfishing_device::tackle_port_w(offs_t offset, u8 data, u8 mem_mask)
{
	// P0.1 drives the reaction motor; its separate battery supply gates the actual output.
	m_motor_drive = BIT(data & mem_mask, 1);
	m_eeprom->cs_w(BIT(data | ~mem_mask, 0));
	update_reaction();
}

void xavix_bassfishing_device::update_reaction()
{
	m_reaction = m_motor_drive && m_powered && m_reaction_battery->read();
}

TIMER_CALLBACK_MEMBER(xavix_bassfishing_device::sample_inputs)
{
	bool const powered = bool(m_power->read());
	u8 const reel = m_reel->read();
	if (powered != m_powered)
	{
		m_powered = powered;
		m_reel_pending = 0;
		m_mcu->power_w(powered);
	}
	else if (powered)
	{
		// Supply quadrature edges, leaving direction, filtering and speed to the ROM.
		m_reel_pending = std::clamp<int>(m_reel_pending + 4 * s8(reel - m_last_reel), -1024, 1024);
		if (m_reel_pending)
		{
			bool const positive = m_reel_pending > 0;
			m_reel_phase = positive ? (((m_reel_phase << 1) & 2) | BIT(~m_reel_phase, 1))
					: ((m_reel_phase >> 1) | (BIT(~m_reel_phase, 0) << 1));
			m_reel_pending += positive ? -1 : 1;
		}
	}
	m_last_reel = reel;
	update_reaction();
}
