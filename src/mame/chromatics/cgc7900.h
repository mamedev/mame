// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_CHROMATICS_CGC7900_H
#define MAME_CHROMATICS_CGC7900_H

#pragma once


#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/ram.h"
#include "machine/i8251.h"
#include "machine/com8116.h"
#include "machine/mm58167.h"
#include "machine/keyboard.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"

#define M68000_TAG      "uh8"
#define INS8251_0_TAG   "uc10"
#define INS8251_1_TAG   "uc8"
#define MM58167_TAG     "uc6"
#define AY8910_TAG      "uc4"
#define K1135A_TAG      "uc11"
#define I8035_TAG       "i8035"
#define AM2910_TAG      "11d"
#define SCREEN_TAG      "screen"

class cgc7900_state : public driver_device
{
public:
	cgc7900_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, M68000_TAG)
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_char_rom(*this, "gfx1")
		, m_chrom_ram(*this, "chrom_ram")
		, m_plane_ram(*this, "plane_ram")
		, m_cmos_ram(*this, "nvram")
		, m_joystick(*this, "JOYSTICK_%c", 'X')
		, m_i8251_0(*this, INS8251_0_TAG)
		, m_i8251_1(*this, INS8251_1_TAG)
	{ }

	required_device<m68000_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_memory_region m_char_rom;
	required_shared_ptr<u16> m_chrom_ram;
	required_shared_ptr<u16> m_plane_ram;
	required_shared_ptr<u16> m_cmos_ram;
	required_ioport_array<3> m_joystick;
	required_device<i8251_device> m_i8251_0;
	required_device<i8251_device> m_i8251_1;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	void cgc7900_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u16 keyboard_r();
	void keyboard_w(u16 data);
	void interrupt_mask_w(u16 data);
	u16 disk_data_r();
	void disk_data_w(u16 data);
	u16 disk_status_r();
	void disk_command_w(u16 data);
	u16 z_mode_r(offs_t offset);
	void z_mode_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void color_status_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 clut_r(offs_t offset);
	void clut_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 overlay_r(offs_t offset);
	void overlay_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 latch_r(offs_t offset);
	void latch_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 sync_r();
	template <int Axis> u16 joystick_r();
	u16 parity_r();
	void parity_w(u16 data);
	void vblank_w(int state);
	u16 int_ack_r(offs_t offset);

	template <unsigned N> void irq(int state) { irq_encoder(N, state); }

	void update_clut();
	void draw_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_overlay(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	/* interrupt state */
	u16 m_int_mask = 0U, m_int_active = 0U;

	/* video state */
	enum
	{
		LATCH_ROLL_BITMAP = 0,
		LATCH_PAN_X,
		LATCH_PAN_Y,
		LATCH_ZOOM,
		LATCH_BLINK_SELECT = 8,
		LATCH_PLANE_SELECT,
		LATCH_PLANE_SWITCH,
		LATCH_COLOR_STATUS_FG,
		LATCH_COLOR_STATUS_BG,
		LATCH_ROLL_OVERLAY
	};

	u16 m_latch[16];

	u16 bitmap_roll() const { return m_latch[LATCH_ROLL_BITMAP] & 0x3ff; }
	u16 pan_x() const { return m_latch[LATCH_PAN_X] & 0x3fc; }
	u16 pan_y() const { return m_latch[LATCH_PAN_Y] & 0x3ff; }
	u8 zoom_x() const { return (m_latch[LATCH_ZOOM] >> 8) & 0x0f; }
	u8 zoom_y() const { return m_latch[LATCH_ZOOM] & 0x0f; }
	u16 overlay_roll() const { return m_latch[LATCH_ROLL_OVERLAY] & 0xfff; }
	bool image_select() const { return BIT(m_latch[LATCH_ROLL_OVERLAY], 13); }
	bool overlay_cursor_blink() const { return BIT(m_latch[LATCH_ROLL_OVERLAY], 14); }
	bool overlay_character_blink() const { return BIT(m_latch[LATCH_ROLL_OVERLAY], 15); }
	u16 m_overlay_ram[0x2000];
	u16 m_clut_ram[0x200];
	rgb_t m_clut[256];
	bool m_blink = false;

	/* joystick state */
	emu_timer *m_adc_timer = nullptr;
	u16 m_adc_data = 0U;
	u8 m_adc_axis = 0U;

	TIMER_CALLBACK_MEMBER(adc_done);

	/* parity state */
	bool m_parity_error = false;

	TIMER_DEVICE_CALLBACK_MEMBER(blink_tick);

	void kbd_put(u8 data);

	void cgc7900(machine_config &config);
	void cgc7900_video(machine_config &config);
	void cgc7900_mem(address_map &map) ATTR_COLD;
	void keyboard_mem(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;
private:
	u16 kbd_mods = 0U;
	u8 kbd_data = 0U;
	bool kbd_ready = false;

	void irq_encoder(int pin, int state);
	void update_interrupts();
};

#endif // MAME_CHROMATICS_CGC7900_H
