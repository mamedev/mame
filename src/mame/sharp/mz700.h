// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Dirk Best
/******************************************************************************
 *  Sharp MZ700
 *
 *  Reference: https://original.sharpmz.org/
 *
 ******************************************************************************/
#ifndef MAME_SHARP_MZ700_H
#define MAME_SHARP_MZ700_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "bus/msx/ctrl/ctrl.h"
#include "imagedev/cassette.h"
#include "machine/bankdev.h"
#include "machine/74145.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/z80pio.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"

class mz_state : public driver_device
{
public:
	mz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_pit(*this, "pit8253")
		, m_ppi(*this, "ppi8255")
		, m_cassette(*this, "cassette")
		, m_centronics(*this, "centronics")
		, m_ram(*this, RAM_TAG)
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_banke(*this, "banke")
		, m_bankf(*this, "bankf")
		, m_ls145(*this, "ls145")
		, m_cursor_timer(*this, "cursor")
	{ }

	void mz700(machine_config &config);

	void init_mz700();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t mz700_e008_r();
	void mz700_e008_w(uint8_t data);
	void mz700_bank_0_w(uint8_t data);
	void mz700_bank_1_w(uint8_t data);
	void mz700_bank_2_w(uint8_t data);
	void mz700_bank_3_w(uint8_t data);
	void mz700_bank_4_w(uint8_t data);
	void mz700_bank_5_w(uint8_t data);
	void mz700_bank_6_w(uint8_t data);
	DECLARE_MACHINE_RESET(mz700);
	uint32_t screen_update_mz700(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(ne556_cursor_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(ne556_other_callback);
	void pit_out0_changed(int state);
	void pit_irq_2(int state);
	uint8_t pio_port_b_r();
	uint8_t pio_port_c_r();
	void pio_port_a_w(uint8_t data);
	void pio_port_c_w(uint8_t data);
	void write_centronics_busy(int state);
	void write_centronics_perror(int state);

	void mz700_banke(address_map &map) ATTR_COLD;
	void mz700_io(address_map &map) ATTR_COLD;
	void mz700_mem(address_map &map) ATTR_COLD;

	int m_mz700 = 0;                /* 1 if running on an mz700 */

	int m_cursor_bit = 0;
	int m_other_timer = 0;

	int m_intmsk = 0;   /* PPI8255 pin PC2 */

	int m_mz700_ram_lock = 0;       /* 1 if ram lock is active */
	int m_mz700_ram_vram = 0;       /* 1 if vram is banked in */

	uint8_t *m_p_chargen = nullptr;

	int m_mz700_mode = 0;           /* 1 if in mz700 mode */
	int m_mz800_ram_lock = 0;       /* 1 if lock is active */
	int m_mz800_ram_monitor = 0;    /* 1 if monitor rom banked in */

	int m_hires_mode = 0;           /* 1 if in 640x200 mode */
	int m_screennum = 0;           /* screen designation */

	int m_centronics_busy = 0;
	int m_centronics_perror = 0;

	uint8_t *m_colorram = nullptr;
	std::unique_ptr<uint8_t[]> m_videoram;
	uint8_t m_speaker_level = 0;
	uint8_t m_prev_state = 0;
	uint8_t m_mz800_palette[4];
	uint8_t m_mz800_palette_bank = 0;

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_device<cassette_image_device> m_cassette;
	optional_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	optional_device<address_map_bank_device> m_banke;
	optional_device<address_map_bank_device> m_bankf;

	required_device<ttl74145_device> m_ls145;
	required_device<timer_device> m_cursor_timer;
};

class mz800_state : public mz_state
{
public:
	mz800_state(const machine_config &mconfig, device_type type, const char *tag)
		: mz_state(mconfig, type, tag)
		, m_joy(*this, "joy%u", 1U)
	{ }

	void mz800(machine_config &config);

	void init_mz800();

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t mz800_bank_0_r();
	void mz800_bank_0_w(uint8_t data);
	uint8_t mz800_bank_1_r();

	uint8_t mz800_crtc_r();
	void mz800_write_format_w(uint8_t data);
	void mz800_read_format_w(uint8_t data);
	void mz800_display_mode_w(uint8_t data);
	void mz800_scroll_border_w(uint8_t data);
	uint8_t mz800_ramdisk_r();
	void mz800_ramdisk_w(uint8_t data);
	void mz800_ramaddr_w(uint8_t data);
	void mz800_palette_w(uint8_t data);
	void mz800_cgram_w(offs_t offset, uint8_t data);
	uint32_t screen_update_mz800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t mz800_z80pio_port_a_r();
	void mz800_z80pio_port_a_w(uint8_t data);
	void pio_port_a_w(uint8_t data);

	void mz800_bankf(address_map &map) ATTR_COLD;
	void mz800_io(address_map &map) ATTR_COLD;
	void mz800_mem(address_map &map) ATTR_COLD;

	required_device_array<msx_general_purpose_port_device, 2> m_joy;

	std::unique_ptr<uint8_t[]> m_cgram;

	uint16_t m_mz800_ramaddr = 0;
};

#endif // MAME_SHARP_MZ700_H
