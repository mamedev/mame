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
DEFINE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODUL, mephisto_display_modul_device, "mdisplay_modul",  "Mephisto Display Modul")


//***************************************************************************
//    IMPLEMENTATION
//***************************************************************************

static INPUT_PORTS_START( mephisto_sensors_board )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_START("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER)	PORT_TOGGLE
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor mephisto_sensors_board_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mephisto_sensors_board );
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
	, m_upd_all_leds(true)
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
//  device_start - device-specific startup
//-------------------------------------------------

void mephisto_board_device::device_start()
{
	save_item(NAME(m_mux));
	save_item(NAME(m_leds));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mephisto_board_device::device_reset()
{
	m_mux = 0x00;
	m_leds = 0x00;
}

void mephisto_board_device::update_leds()
{
	for (int i=0; i<8; i++)
		if (m_upd_all_leds || !BIT(m_mux, i))
			for (int j=0; j<8; j++)
				machine().output().set_led_value(i*8 + j, !BIT(m_mux, i) ? BIT(m_leds, j) : 0);

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

WRITE8_MEMBER( mephisto_board_device::mux_upd_w )
{
	m_mux = data;
	update_leds();
}

WRITE8_MEMBER( mephisto_board_device::led_upd_w )
{
	m_leds = data;
	update_leds();
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

MACHINE_CONFIG_MEMBER( mephisto_display_modul_device::device_add_mconfig )
	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(16*6, 9*2)
	MCFG_SCREEN_VISIBLE_AREA(0, 16*6-1, 0, 9*2-3)
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(mephisto_display_modul_device, lcd_palette)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 16)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 3250)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


PALETTE_INIT_MEMBER(mephisto_display_modul_device,lcd_palette)
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
		m_lcdc->write(space, BIT(data, 0), m_latch);

	m_beeper->set_state(BIT(data, 2) | BIT(data, 3));

	m_ctrl = data;
}
