// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*******************************************************************************

    Mephisto Modular Sensors Board

*******************************************************************************/

#ifndef MAME_HEGENERGLASER_MMBOARD_H
#define MAME_HEGENERGLASER_MMBOARD_H

#pragma once

#include "machine/sensorboard.h"
#include "video/pwm.h"


// ======================> mephisto_board_device

class mephisto_board_device : public device_t
{
public:
	// construction/destruction
	mephisto_board_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	void set_disable_leds(bool disable_leds) { m_disable_leds = disable_leds; }
	void set_delay(attotime sensordelay)     { m_sensordelay = sensordelay; }

	sensorboard_device *get() { return m_board; }

	u8 input_r();
	void led_w(u8 data);
	u8 mux_r();
	void mux_w(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void set_config(machine_config &config, sensorboard_device::sb_type board_type);
	void refresh_leds_w(offs_t offset, u8 data);
	void update_led_pwm() { m_led_pwm->matrix(~m_mux, m_led_data); }

	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	output_finder<64> m_led_out;

	attotime m_sensordelay;
	bool m_disable_leds;
	u8 m_led_data;
	u8 m_mux;
};


// ======================> mephisto_sensors_board_device

class mephisto_sensors_board_device : public mephisto_board_device
{
public:
	// construction/destruction
	mephisto_sensors_board_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> mephisto_buttons_board_device

class mephisto_buttons_board_device : public mephisto_board_device
{
public:
	// construction/destruction
	mephisto_buttons_board_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MEPHISTO_SENSORS_BOARD, mephisto_sensors_board_device)
DECLARE_DEVICE_TYPE(MEPHISTO_BUTTONS_BOARD, mephisto_buttons_board_device)


#endif // MAME_HEGENERGLASER_MMBOARD_H
