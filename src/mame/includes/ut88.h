// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/ut88.h
 *
 ****************************************************************************/

#ifndef UT88_H_
#define UT88_H_

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/dac.h"
#include "sound/wave.h"
#include "machine/i8255.h"
#include "imagedev/cassette.h"


class ut88_state : public driver_device
{
public:
	enum
	{
		TIMER_RESET,
		TIMER_UPDATE_DISPLAY
	};

	ut88_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cassette(*this, "cassette"),
		m_ppi(*this, "ppi8255"),
		m_dac(*this, "dac"),
		m_p_videoram(*this, "p_videoram"),
		m_region_maincpu(*this, "maincpu"),
		m_region_proms(*this, "proms"),
		m_bank1(*this, "bank1"),
		m_io_line0(*this, "LINE0"),
		m_io_line1(*this, "LINE1"),
		m_io_line2(*this, "LINE2"),
		m_io_line3(*this, "LINE3"),
		m_io_line4(*this, "LINE4"),
		m_io_line5(*this, "LINE5"),
		m_io_line6(*this, "LINE6"),
		m_io_line7(*this, "LINE7"),
		m_io_line8(*this, "LINE8") ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_device<cassette_image_device> m_cassette;
	optional_device<i8255_device> m_ppi;
	optional_device<dac_bit_interface> m_dac;
	uint8_t ut88_keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ut88_keyboard_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ut88_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ut88_tape_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ut88mini_keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ut88mini_write_led(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ut88_8255_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ut88_8255_portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ut88_8255_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	optional_shared_ptr<uint8_t> m_p_videoram;
	int m_keyboard_mask;
	int m_lcd_digit[6];
	void init_ut88();
	void init_ut88mini();
	void machine_reset_ut88();
	void video_start_ut88();
	void machine_start_ut88mini();
	void machine_reset_ut88mini();
	uint32_t screen_update_ut88(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_proms;
	optional_memory_bank m_bank1;
	required_ioport m_io_line0;
	required_ioport m_io_line1;
	required_ioport m_io_line2;
	optional_ioport m_io_line3;
	optional_ioport m_io_line4;
	optional_ioport m_io_line5;
	optional_ioport m_io_line6;
	optional_ioport m_io_line7;
	optional_ioport m_io_line8;
	required_device<cpu_device> m_maincpu;
	optional_device<gfxdecode_device> m_gfxdecode;
	optional_device<palette_device> m_palette;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/*----------- defined in video/ut88.c -----------*/

extern const gfx_layout ut88_charlayout;


#endif /* UT88_H_ */
