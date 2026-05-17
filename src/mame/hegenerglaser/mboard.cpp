// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/*******************************************************************************

Hegener + Glaser Mephisto Sensors Board, for modular chesscomputers
- Modular
- Muenchen
- Exclusive

This device can also apply to non-modular boards if I/O is similar. The Bavaria
board is not emulated here, additional handlers for it are in the driver.

*******************************************************************************/

#include "emu.h"
#include "mboard.h"


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

	m_board->set_delay(m_sensordelay);

	// zerofill
	m_mux = 0xff;
	m_led_data = 0;
	m_led_latch = 0;
	m_cb_latch = 0xff;

	m_data = 0;
	m_row_le = 0;
	m_ldc_le = 0;
	m_ldc_en = 0;
	m_cb_en = 0;

	// register for savestates
	save_item(NAME(m_mux));
	save_item(NAME(m_led_data));
	save_item(NAME(m_led_latch));
	save_item(NAME(m_cb_latch));

	save_item(NAME(m_data));
	save_item(NAME(m_row_le));
	save_item(NAME(m_ldc_le));
	save_item(NAME(m_ldc_en));
	save_item(NAME(m_cb_en));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mephisto_board_device::device_reset()
{
	m_mux = 0xff;
	m_led_data = 0;
	m_led_latch = 0;
	m_cb_latch = 0xff;

	update_led_pwm();
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

// misc

void mephisto_board_device::refresh_leds_w(offs_t offset, u8 data)
{
	if (!m_disable_leds)
		m_led_out[(offset >> 6 & 7) | (offset & 7) << 3] = data;
}


// high level interface

// note: the 2 interfaces are identical hardware, the high level one just assumes
// automatic CS and RW handling via MAME addressmaps

u8 mephisto_board_device::input_r()
{
	u8 data = 0xff;

	for (int i = 0; i < 8; i++)
		if (!BIT(m_mux, i))
			data &= ~m_board->read_rank(i);

	return data;
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


// low level interface

void mephisto_board_device::update_board()
{
	if (m_row_le)
		mux_w(m_data);

	if (m_ldc_le)
		m_led_latch = m_data;

	if (m_ldc_en)
		m_cb_latch = input_r();
	else
		led_w(m_led_latch);
}

u8 mephisto_board_device::data_r()
{
	if (m_cb_en)
		return 0xff;

	return m_ldc_en ? input_r() : m_cb_latch;
}

void mephisto_board_device::data_w(u8 data)
{
	m_data = data;
	update_board();
}

void mephisto_board_device::row_le_w(int state)
{
	// 74373(1) /LE
	m_row_le = state ? 1 : 0;
	update_board();
}

void mephisto_board_device::ldc_le_w(int state)
{
	// 74373(2) /LE
	m_ldc_le = state ? 1 : 0;
	update_board();
}

void mephisto_board_device::ldc_en_w(int state)
{
	// 74373(2) /OE & 74373(3) /LE
	state = state ? 1 : 0;

	// refresh inputs
	if (!state && m_ldc_en)
		m_cb_latch = input_r();

	m_ldc_en = state;
	update_board();
}

void mephisto_board_device::cb_en_w(int state)
{
	// 74373(3) /OE
	m_cb_en = state ? 1 : 0;
}
