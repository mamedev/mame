// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Mephisto Sensors Board

*********************************************************************/

#ifndef MAME_MACHINE_MMBOARD_H
#define MAME_MACHINE_MMBOARD_H

#pragma once

#include "machine/sensorboard.h"
#include "video/pwm.h"


// ======================> mephisto_board_device

class mephisto_board_device : public device_t
{
public:
	// construction/destruction
	mephisto_board_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_disable_leds(int disable_leds) { m_disable_leds = disable_leds; }
	void set_delay(attotime sensordelay)    { m_sensordelay = sensordelay; }

	sensorboard_device *get() { return m_board; }

	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_READ8_MEMBER(mux_r);
	DECLARE_WRITE8_MEMBER(mux_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void set_config(machine_config &config, sensorboard_device::sb_type board_type);
	DECLARE_WRITE8_MEMBER(refresh_leds_w);
	void update_led_pwm() { m_led_pwm->matrix(~m_mux, m_led_data); }

	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	attotime                 m_sensordelay;
	output_finder<64>        m_led_out;
	bool                     m_disable_leds;
	uint8_t                  m_led_data;
	uint8_t                  m_mux;
};

// ======================> mephisto_sensors_board_device

class mephisto_sensors_board_device : public mephisto_board_device
{
public:
	// construction/destruction
	mephisto_sensors_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};


// ======================> mephisto_buttons_board_device

class mephisto_buttons_board_device : public mephisto_board_device
{
public:
	// construction/destruction
	mephisto_buttons_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};


// device type definition
DECLARE_DEVICE_TYPE(MEPHISTO_SENSORS_BOARD, mephisto_sensors_board_device)
DECLARE_DEVICE_TYPE(MEPHISTO_BUTTONS_BOARD, mephisto_buttons_board_device)


#endif // MAME_MACHINE_MMBOARD_H
