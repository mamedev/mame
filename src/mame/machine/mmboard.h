// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Mephisto Modular

*********************************************************************/

#ifndef MAME_MACHINE_MMBOARD_H
#define MAME_MACHINE_MMBOARD_H

#pragma once


#include "sound/beep.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mephisto_board_device

class mephisto_board_device : public device_t
{
public:
	// construction/destruction
	mephisto_board_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_disable_leds(int _disable_leds) { m_disable_leds = _disable_leds; }

	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_READ8_MEMBER(mux_r);
	DECLARE_WRITE8_MEMBER(mux_w);

	TIMER_CALLBACK_MEMBER(leds_update_callback);
	TIMER_CALLBACK_MEMBER(leds_refresh_callback);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_ioport_array<8> m_sensors;
	output_finder<64>        m_led;
	emu_timer *              m_leds_update_timer;
	emu_timer *              m_leds_refresh_timer;
	bool                     m_disable_leds;
	uint8_t                  m_mux;
	uint8_t                  m_leds;
	uint8_t                  m_leds_state[64];
};

// ======================> mephisto_sensors_board_device

class mephisto_sensors_board_device : public mephisto_board_device
{
public:
	// construction/destruction
	mephisto_sensors_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
};


// ======================> mephisto_buttons_board_device

class mephisto_buttons_board_device : public mephisto_board_device
{
public:
	// construction/destruction
	mephisto_buttons_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
};


// ======================> mephisto_display_modul_device

class mephisto_display_modul_device : public device_t
{
public:
	// construction/destruction
	mephisto_display_modul_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER(latch_w);
	DECLARE_WRITE8_MEMBER(io_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void lcd_palette(palette_device &palette) const;

private:
	optional_device<hd44780_device> m_lcdc;
	required_device<beep_device> m_beeper;
	uint8_t m_latch;
	uint8_t m_ctrl;
};


// device type definition
DECLARE_DEVICE_TYPE(MEPHISTO_SENSORS_BOARD, mephisto_sensors_board_device)
DECLARE_DEVICE_TYPE(MEPHISTO_BUTTONS_BOARD, mephisto_buttons_board_device)
DECLARE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODUL, mephisto_display_modul_device)


#endif // MAME_MACHINE_MMBOARD_H
