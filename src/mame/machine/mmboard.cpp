// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

Mephisto Sensors Board emulation
- Modular
- Muenchen
- Exclusive

Mephisto Display Modul emulation

This device can also apply to non-modular boards if I/O is same

*********************************************************************/

#include "emu.h"
#include "mmboard.h"
#include "sound/volt_reg.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MEPHISTO_SENSORS_BOARD, mephisto_sensors_board_device, "msboard", "Mephisto Sensors Board")
DEFINE_DEVICE_TYPE(MEPHISTO_BUTTONS_BOARD, mephisto_buttons_board_device, "mbboard", "Mephisto Buttons Board")
DEFINE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODUL, mephisto_display_modul_device, "mdisplay_modul", "Mephisto Display Modul")


//***************************************************************************
//    IMPLEMENTATION
//***************************************************************************

//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void mephisto_sensors_board_device::device_add_mconfig(machine_config &config)
{
	set_config(config, sensorboard_device::MAGNETS);
}

void mephisto_buttons_board_device::device_add_mconfig(machine_config &config)
{
	set_config(config, sensorboard_device::BUTTONS);
}

void mephisto_board_device::set_config(machine_config &config, sensorboard_device::sb_type board_type)
{
	SENSORBOARD(config, m_board).set_type(board_type);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));

	PWM_DISPLAY(config, m_led_pwm).set_size(8, 8);
	m_led_pwm->output_x().set(FUNC(mephisto_board_device::refresh_leds_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mephisto_board_device - constructor
//-------------------------------------------------

mephisto_board_device::mephisto_board_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_board(*this, "board")
	, m_led_pwm(*this, "led_pwm")
	, m_sensordelay(attotime::from_msec(150))
	, m_led_out(*this, "led%u", 0U)
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
	m_led_out.resolve();

	save_item(NAME(m_mux));
	save_item(NAME(m_led_data));

	m_board->set_delay(m_sensordelay);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mephisto_board_device::device_reset()
{
	m_mux = 0x00;
	m_led_data = 0x00;
	update_led_pwm();
}

WRITE8_MEMBER( mephisto_board_device::refresh_leds_w )
{
	if (!m_disable_leds)
		m_led_out[(offset >> 6 & 7) | (offset & 7) << 3] = data;
}

READ8_MEMBER( mephisto_board_device::input_r )
{
	uint8_t data = 0xff;

	for (int i = 0; i < 8; i++)
		if (!BIT(m_mux, i))
			data &= ~m_board->read_rank(i);

	return data;
}

READ8_MEMBER( mephisto_board_device::mux_r )
{
	return m_mux;
}

WRITE8_MEMBER( mephisto_board_device::mux_w )
{
	m_mux = data;
	update_led_pwm();
}

WRITE8_MEMBER( mephisto_board_device::led_w )
{
	m_led_data = data;
	update_led_pwm();
}



//-------------------------------------------------
//  mephisto_display_modul_device - constructor
//-------------------------------------------------

mephisto_display_modul_device::mephisto_display_modul_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEPHISTO_DISPLAY_MODUL, tag, owner, clock)
	, m_lcdc(*this, "hd44780")
	, m_dac(*this, "dac")
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
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_BINARY_WEIGHTED_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
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

	m_dac->write(data >> 2 & 3);

	m_ctrl = data;
}
