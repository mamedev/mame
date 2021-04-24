// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

Hegener + Glaser Mephisto Sensors Board, for modular chesscomputers
- Modular
- Muenchen
- Exclusive

This device can also apply to non-modular boards if I/O is similar
Bavaria board is not emulated here, additional handlers for it are in the driver.

*********************************************************************/

#include "emu.h"
#include "mmboard.h"


DEFINE_DEVICE_TYPE(MEPHISTO_SENSORS_BOARD, mephisto_sensors_board_device, "msboard", "Mephisto Sensors Board")
DEFINE_DEVICE_TYPE(MEPHISTO_BUTTONS_BOARD, mephisto_buttons_board_device, "mbboard", "Mephisto Buttons Board")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

mephisto_board_device::mephisto_board_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_board(*this, "board")
	, m_led_pwm(*this, "led_pwm")
	, m_led_out(*this, "led%u", 0U)
	, m_sensordelay(attotime::from_msec(150))
	, m_disable_leds(false)
{
}

mephisto_sensors_board_device::mephisto_sensors_board_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mephisto_board_device(mconfig, MEPHISTO_SENSORS_BOARD, tag, owner, clock)
{
}

mephisto_buttons_board_device::mephisto_buttons_board_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mephisto_board_device(mconfig, MEPHISTO_BUTTONS_BOARD, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_add_mconfig
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


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mephisto_board_device::device_start()
{
	m_led_out.resolve();

	m_mux = 0xff;
	m_led_data = 0;

	save_item(NAME(m_mux));
	save_item(NAME(m_led_data));

	m_board->set_delay(m_sensordelay);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mephisto_board_device::device_reset()
{
	m_mux = 0xff;
	m_led_data = 0x00;
	update_led_pwm();
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

void mephisto_board_device::refresh_leds_w(offs_t offset, u8 data)
{
	if (!m_disable_leds)
		m_led_out[(offset >> 6 & 7) | (offset & 7) << 3] = data;
}

u8 mephisto_board_device::input_r()
{
	u8 data = 0xff;

	for (int i = 0; i < 8; i++)
		if (!BIT(m_mux, i))
			data &= ~m_board->read_rank(i);

	return data;
}

u8 mephisto_board_device::mux_r()
{
	return m_mux;
}

void mephisto_board_device::mux_w(u8 data)
{
	m_mux = data;
	update_led_pwm();
}

void mephisto_board_device::led_w(u8 data)
{
	m_led_data = data;
	update_led_pwm();
}
