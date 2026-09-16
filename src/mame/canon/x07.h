// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*********************************************************************

    includes/x07.h

*********************************************************************/
#ifndef MAME_CANON_X07_H
#define MAME_CANON_X07_H

#pragma once

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/z80/nsc800.h"
#include "imagedev/cassette.h"
#include "imagedev/printer.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/beep.h"

#include "emupal.h"

#include "formats/x07_cas.h"

class x07_state : public driver_device
{
public:
	x07_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_printer(*this, "printer")
		, m_beep(*this, "beeper")
		, m_ram(*this, RAM_TAG)
		, m_nvram1(*this, "nvram1")
		, m_nvram2(*this, "nvram2")
		, m_cassette(*this, "cassette")
		, m_card(*this, "cardslot")
		, m_io_keyboard(*this, { "S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8", "BZ", "A1" })
		, m_warm_start(1)
	{ }

	void x07(machine_config &config) ATTR_COLD;

	void init_x07() ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER( kb_keys );
	DECLARE_INPUT_CHANGED_MEMBER( kb_func_keys );
	DECLARE_INPUT_CHANGED_MEMBER( kb_break );
	DECLARE_INPUT_CHANGED_MEMBER( kb_update_udk );

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t x07_io_r(offs_t offset);
	void x07_io_w(offs_t offset, uint8_t data);

	void nvram_init(nvram_device &nvram, void *data, size_t size);

	void t6834_cmd(uint8_t cmd);
	void t6834_r();
	void t6834_w();
	void cassette_w();
	void cassette_r();
	void printer_w();
	void kb_irq();
	void cassette_load();
	void cassette_save();
	void receive_bit(int bit);

	void t6834_set_audio();
	void t6834_reset_audio();

	inline uint8_t get_char(uint16_t pos);
	inline uint8_t kb_get_index(uint8_t char_code);
	inline void draw_char(uint8_t x, uint8_t y, uint8_t char_pos);
	inline void draw_point(uint8_t x, uint8_t y, uint8_t color);
	inline void draw_udk();

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( card_load );

	TIMER_CALLBACK_MEMBER(cassette_tick);
	TIMER_CALLBACK_MEMBER(cassette_poll);
	TIMER_CALLBACK_MEMBER(rsta_clear);
	TIMER_CALLBACK_MEMBER(rstb_clear);
	TIMER_CALLBACK_MEMBER(audio_tick);
	TIMER_CALLBACK_MEMBER(click_stop);
	TIMER_DEVICE_CALLBACK_MEMBER(blink_timer);

	void x07_io(address_map &map) ATTR_COLD;
	void x07_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<printer_image_device> m_printer;
	required_device<beep_device> m_beep;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram1;
	required_device<nvram_device> m_nvram2;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_card;
	required_ioport_array<10> m_io_keyboard;

	/* general */
	uint8_t m_sleep = 0;
	uint8_t m_warm_start;
	uint8_t m_t6834_ram[0x800];
	uint8_t m_regs_r[8];
	uint8_t m_regs_w[8];
	uint8_t m_alarm[8];

	struct fifo_buffer
	{
		uint8_t data[0x100];
		uint8_t read = 0;
		uint8_t write = 0;
	};
	fifo_buffer m_in;
	fifo_buffer m_out;

	uint8_t m_udk_on = 0;
	uint8_t m_draw_udk = 0;
	uint8_t m_sp_on = 0;
	uint8_t m_font_code = 0;
	emu_timer *m_rsta_clear = nullptr;
	emu_timer *m_rstb_clear = nullptr;
	emu_timer *m_audio_tick = nullptr;
	emu_timer *m_click_stop = nullptr;

	/* LCD */
	uint8_t m_lcd_on = 0;
	uint8_t m_lcd_map[32][120]{};
	uint8_t m_scroll_min = 0;
	uint8_t m_scroll_max = 0;
	uint8_t m_blink = 0;

	struct lcd_position
	{
		uint8_t x = 0;
		uint8_t y = 0;
		uint8_t on = 0;
	};
	lcd_position m_cursor;

	/* keyboard */
	uint8_t m_kb_on = 0;
	uint8_t m_click_on = 1;           //key click enabled by default
	uint8_t m_repeat_key = 0;         //not supported
	uint8_t m_kb_size = 0;

	/* cassette */
	uint8_t  m_cass_motor = 0;
	uint8_t  m_cass_data = 0;
	uint32_t m_cass_clk = 0;
	uint8_t  m_bit_count = 0;
	int    m_cass_state = 0;
	emu_timer *m_cass_poll = nullptr;
	emu_timer *m_cass_tick = nullptr;

	/* printer */
	uint8_t m_prn_sendbit = 0;
	uint8_t m_prn_char_code = 0;
	uint8_t m_prn_buffer[0x100]{};
	uint8_t m_prn_size = 0;
	void x07_palette(palette_device &palette) const;
};

#endif // MAME_CANON_X07_H
