// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Mephisto Sensors Board emulation
    Mephisto Display Modul emulation

*********************************************************************/

#include "emu.h"
#include "mmboard.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MEPHISTO_SENSORS_BOARD, mephisto_sensors_board_device, "msboard", "Mephisto Sensors Board")
DEFINE_DEVICE_TYPE(MEPHISTO_BUTTONS_BOARD, mephisto_buttons_board_device, "mbboard", "Mephisto Buttons Board")
DEFINE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODUL, mephisto_display_modul_device, "mdisplay_modul",  "Mephisto Display Modul")


//***************************************************************************
//    IMPLEMENTATION
//***************************************************************************

static INPUT_PORTS_START( mephisto_sensors_board )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_START("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)   PORT_TOGGLE
INPUT_PORTS_END

static INPUT_PORTS_START( mephisto_buttons_board )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_START("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor mephisto_sensors_board_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mephisto_sensors_board );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor mephisto_buttons_board_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mephisto_buttons_board );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mephisto_board_device - constructor
//-------------------------------------------------

mephisto_board_device::mephisto_board_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_sensors(*this, "IN.%u", 0)
	, m_led(*this, "led%u", 0U)
	, m_disable_leds(false)
{
}

//-------------------------------------------------
//  mephisto_sensors_board_device - constructor
//-------------------------------------------------

mephisto_sensors_board_device::mephisto_sensors_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mephisto_board_device(mconfig, MEPHISTO_SENSORS_BOARD, tag, owner, clock)
{
}
//-------------------------------------------------
//  mephisto_buttons_board_device - constructor
//-------------------------------------------------

mephisto_buttons_board_device::mephisto_buttons_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mephisto_board_device(mconfig, MEPHISTO_BUTTONS_BOARD, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mephisto_board_device::device_start()
{
	m_led.resolve();
	m_leds_update_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mephisto_board_device::leds_update_callback), this));
	m_leds_refresh_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mephisto_board_device::leds_refresh_callback), this));
	m_leds_update_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
	m_leds_refresh_timer->adjust(attotime::from_hz(5), 0, attotime::from_hz(5));

	save_item(NAME(m_mux));
	save_item(NAME(m_leds));
	save_item(NAME(m_leds_state));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mephisto_board_device::device_reset()
{
	m_mux = 0x00;
	m_leds = 0x00;
	memset(m_leds_state, 0, sizeof(m_leds_state));
}

TIMER_CALLBACK_MEMBER(mephisto_board_device::leds_update_callback)
{
	for (int i=0; i<8; i++)
		for (int j=0; j<8; j++)
		{
			if (!m_leds_state[i*8 + j] && !BIT(m_mux, i) && BIT(m_leds, j))
				m_leds_state[i*8 + j] = 2;
		}
}

TIMER_CALLBACK_MEMBER(mephisto_board_device::leds_refresh_callback)
{
	for (int i=0; i<8; i++)
		for (int j=0; j<8; j++)
		{
			if (!m_disable_leds)
				m_led[i*8 + j] = (m_leds_state[i*8 + j] > 1) ? 1 : 0;

			if (m_leds_state[i*8 + j])
				m_leds_state[i*8 + j]--;
		}
}

READ8_MEMBER( mephisto_board_device::input_r )
{
	uint8_t data = 0xff;

	for (int i=0; i<8; i++)
		if (!BIT(m_mux, i))
			data &= m_sensors[i]->read();

	return data;
}

READ8_MEMBER( mephisto_board_device::mux_r )
{
	return m_mux;
}

WRITE8_MEMBER( mephisto_board_device::mux_w )
{
	m_mux = data;
}

WRITE8_MEMBER( mephisto_board_device::led_w )
{
	m_leds = data;
}



//-------------------------------------------------
//  mephisto_display_modul_device - constructor
//-------------------------------------------------

mephisto_display_modul_device::mephisto_display_modul_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEPHISTO_DISPLAY_MODUL, tag, owner, clock)
	, m_lcdc(*this, "hd44780")
	, m_beeper(*this, "beeper")
{
}

//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void mephisto_display_modul_device::device_add_mconfig(machine_config &config)
{
	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_size(16*6, 9*2);
	screen.set_visarea(0, 16*6-1, 0, 9*2-3);
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");
	PALETTE(config, "palette", FUNC(mephisto_display_modul_device::lcd_palette), 2);

	HD44780(config, m_lcdc, 0);
	m_lcdc->set_lcd_size(2, 16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 3250).add_route(ALL_OUTPUTS, "mono", 1.0);
}


void mephisto_display_modul_device::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mephisto_display_modul_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_ctrl));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mephisto_display_modul_device::device_reset()
{
	m_latch = 0;
	m_ctrl = 0;
}

WRITE8_MEMBER(mephisto_display_modul_device::latch_w)
{
	m_latch = data;
}

WRITE8_MEMBER(mephisto_display_modul_device::io_w)
{
	if (BIT(data, 1) && !BIT(m_ctrl, 1))
		m_lcdc->write(BIT(data, 0), m_latch);

	m_beeper->set_state(BIT(data, 2) | BIT(data, 3));

	m_ctrl = data;
}
