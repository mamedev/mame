// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Alesis HR-16 and SR-16 drum machines

****************************************************************************/

#pragma once

#ifndef _ALESIS_H_
#define _ALESIS_H_

#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/hd44780.h"
#include "imagedev/cassette.h"
#include "rendlay.h"

#define MCFG_ALESIS_DM3AG_ADD(_tag,_clock) \
	MCFG_DEVICE_ADD( _tag, ALESIS_DM3AG, _clock )


// ======================> alesis_dm3ag_device

class alesis_dm3ag_device : public device_t
{
public:
	// construction/destruction
	alesis_dm3ag_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device interface
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	static const device_timer_id TIMER_DAC_UPDATE = 1;
	required_device<dac_word_interface> m_dac;
	required_region_ptr<int8_t> m_samples;

	emu_timer * m_dac_update_timer;
	bool        m_output_active;
	int         m_count;
	int         m_shift;
	uint32_t      m_cur_sample;
	uint8_t       m_cmd[5];
};


// ======================> alesis_state

class alesis_state : public driver_device
{
public:
	alesis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_lcdc(*this, "hd44780"),
			m_cassette(*this, "cassette"),
			m_maincpu(*this, "maincpu"),
			m_col1(*this, "COL1"),
			m_col2(*this, "COL2"),
			m_col3(*this, "COL3"),
			m_col4(*this, "COL4"),
			m_col5(*this, "COL5"),
			m_col6(*this, "COL6"),
			m_select(*this, "SELECT")
	{ }

	required_device<hd44780_device> m_lcdc;
	optional_device<cassette_image_device> m_cassette;

	void palette_init_alesis(palette_device &palette);
	virtual void machine_reset() override;

	void update_lcd_symbols(bitmap_ind16 &bitmap, uint8_t pos, uint8_t y, uint8_t x, int state);
	void init_hr16();
	void led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mmt8_led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mmt8_led_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void track_led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kb_matrix_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t kb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t p3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mmt8_p3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mmt8_p3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sr16_lcd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	HD44780_PIXEL_UPDATE(sr16_pixel_update);

private:
	uint8_t       m_kb_matrix;
	uint8_t       m_leds;
	uint8_t       m_lcd_digits[5];
	required_device<cpu_device> m_maincpu;
	required_ioport m_col1;
	required_ioport m_col2;
	required_ioport m_col3;
	required_ioport m_col4;
	required_ioport m_col5;
	required_ioport m_col6;
	optional_ioport m_select;
};

// device type definition
extern const device_type ALESIS_DM3AG;

#endif  // _ALESIS_H_
