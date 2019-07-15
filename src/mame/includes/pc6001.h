// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_INCLUDES_PC6001_H
#define MAME_INCLUDES_PC6001_H

#pragma once


#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/upd7752.h"
//#include "sound/2203intf.h"
#include "video/mc6847.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "speaker.h"
#include "screen.h"

#include "formats/p6001_cas.h"

class pc6001_state : public driver_device
{
public:
	pc6001_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ppi(*this, "ppi8255"),
		m_ram(*this, "ram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_cassette(*this, "cassette"),
		m_cas_hack(*this, "cas_hack"),
		m_cart(*this, "cartslot"),
		m_region_maincpu(*this, "maincpu"),
		m_region_gfx1(*this, "gfx1"),
		m_io_mode4_dsw(*this, "MODE4_DSW"),
		m_io_p1(*this, "P1"),
		m_io_p2(*this, "P2"),
		m_io_keys(*this, "key%u", 1U),
		m_io_key_modifiers(*this, "key_modifiers"),
		m_bank1(*this, "bank1"),
		m_palette(*this, "palette")
	{ }

	DECLARE_WRITE8_MEMBER(system_latch_w);
	DECLARE_READ8_MEMBER(nec_ppi8255_r);
	DECLARE_WRITE8_MEMBER(nec_ppi8255_w);

	void pc6001_palette(palette_device &palette) const;

	uint32_t screen_update_pc6001(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vrtc_irq);
	TIMER_CALLBACK_MEMBER(audio_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(cassette_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);

	DECLARE_READ8_MEMBER(ppi_porta_r);
	DECLARE_WRITE8_MEMBER(ppi_porta_w);
	DECLARE_READ8_MEMBER(ppi_portb_r);
	DECLARE_WRITE8_MEMBER(ppi_portb_w);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);
	DECLARE_READ8_MEMBER(ppi_portc_r);

	IRQ_CALLBACK_MEMBER(irq_callback);

	void pc6001(machine_config &config);
	void pc6001_io(address_map &map);
	void pc6001_map(address_map &map);
protected:
	required_device<i8255_device> m_ppi;
	optional_shared_ptr<uint8_t> m_ram;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<cassette_image_device> m_cassette;
	optional_device<generic_slot_device> m_cas_hack;
	required_device<generic_slot_device> m_cart;
	required_memory_region m_region_maincpu;
	required_memory_region m_region_gfx1;
	required_ioport m_io_mode4_dsw;
	required_ioport m_io_p1;
	required_ioport m_io_p2;
	required_ioport_array<3> m_io_keys;
	required_ioport m_io_key_modifiers;
	required_memory_bank m_bank1;
	required_device<palette_device> m_palette;

	memory_region *m_cart_rom;
	uint8_t m_timer_irq_vector;
	uint16_t m_timer_hz_div;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	// i/o functions
	uint8_t check_joy_press();
	uint8_t check_keyboard_press();
	inline void cassette_latch_control(bool new_state);
	inline void ppi_control_hack_w(uint8_t data);
	inline void set_timer_divider(uint8_t data);
	inline void set_videoram_bank(uint32_t offs);
	inline void set_maincpu_irq_line(uint8_t vector_num);

	// video functions
	void draw_gfx_mode4(bitmap_ind16 &bitmap,const rectangle &cliprect,int attr);
	void draw_bitmap_2bpp(bitmap_ind16 &bitmap,const rectangle &cliprect, int attr);
	void draw_tile_3bpp(bitmap_ind16 &bitmap,const rectangle &cliprect,int x,int y,int tile,int attr);
	void draw_tile_text(bitmap_ind16 &bitmap,const rectangle &cliprect,int x,int y,int tile,int attr,int has_mc6847);
	void draw_border(bitmap_ind16 &bitmap,const rectangle &cliprect,int attr,int has_mc6847);
	void pc6001_screen_draw(bitmap_ind16 &bitmap,const rectangle &cliprect, int has_mc6847);

	emu_timer *m_timer_irq_timer;
	uint8_t *m_video_ram;
	uint8_t m_irq_vector;
	uint8_t m_cas_switch;
	uint8_t m_sys_latch;
	uint32_t m_cas_offset;
	uint32_t m_cas_maxsize;
	uint8_t m_bank_opt;
	uint8_t m_timer_irq_mask;
	uint8_t m_timer_irq_mask2;
	uint8_t m_port_c_8255;
	uint8_t m_cur_keycode;

private:
	uint32_t m_old_key1;
	uint32_t m_old_key2;
	uint32_t m_old_key3;

};


class pc6001mk2_state : public pc6001_state
{
public:
	pc6001mk2_state(const machine_config &mconfig, device_type type, const char *tag) :
		pc6001_state(mconfig, type, tag),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_bank5(*this, "bank5"),
		m_bank6(*this, "bank6"),
		m_bank7(*this, "bank7"),
		m_bank8(*this, "bank8")
	{ }

	DECLARE_READ8_MEMBER(mk2_bank_r0_r);
	DECLARE_READ8_MEMBER(mk2_bank_r1_r);
	DECLARE_READ8_MEMBER(mk2_bank_w0_r);
	DECLARE_WRITE8_MEMBER(mk2_bank_r0_w);
	DECLARE_WRITE8_MEMBER(mk2_bank_r1_w);
	DECLARE_WRITE8_MEMBER(mk2_bank_w0_w);
	DECLARE_WRITE8_MEMBER(mk2_opt_bank_w);
	DECLARE_WRITE8_MEMBER(mk2_work_ram0_w);
	DECLARE_WRITE8_MEMBER(mk2_work_ram1_w);
	DECLARE_WRITE8_MEMBER(mk2_work_ram2_w);
	DECLARE_WRITE8_MEMBER(mk2_work_ram3_w);
	DECLARE_WRITE8_MEMBER(mk2_work_ram4_w);
	DECLARE_WRITE8_MEMBER(mk2_work_ram5_w);
	DECLARE_WRITE8_MEMBER(mk2_work_ram6_w);
	DECLARE_WRITE8_MEMBER(mk2_work_ram7_w);
	DECLARE_WRITE8_MEMBER(necmk2_ppi8255_w);
	DECLARE_WRITE8_MEMBER(mk2_system_latch_w);
	DECLARE_WRITE8_MEMBER(mk2_vram_bank_w);
	DECLARE_WRITE8_MEMBER(mk2_col_bank_w);
	DECLARE_WRITE8_MEMBER(mk2_0xf3_w);
	DECLARE_WRITE8_MEMBER(mk2_timer_adj_w);
	DECLARE_WRITE8_MEMBER(mk2_timer_irqv_w);

	void pc6001mk2_palette(palette_device &palette) const;
	void pc6001mk2(machine_config &config);

	uint32_t screen_update_pc6001mk2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pc6001mk2_io(address_map &map);
	void pc6001mk2_map(address_map &map);
protected:
	uint8_t m_bgcol_bank;
	uint8_t m_gfx_bank_on;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_memory_bank m_bank5;
	required_memory_bank m_bank6;
	required_memory_bank m_bank7;
	required_memory_bank m_bank8;
	inline void refresh_crtc_params();

	virtual void video_start() override;
	virtual void machine_reset() override;

private:
	uint8_t m_bank_r0;
	uint8_t m_bank_r1;
	uint8_t m_bank_w;
	uint8_t m_ex_vram_bank;
	uint8_t m_exgfx_text_mode;
	uint32_t m_cgrom_bank_addr;
	uint8_t m_exgfx_bitmap_mode;
	uint8_t m_exgfx_2bpp_mode;

	void vram_bank_change(uint8_t vram_bank);
};

class pc6601_state : public pc6001mk2_state
{
public:
	pc6601_state(const machine_config &mconfig, device_type type, const char *tag) :
		pc6001mk2_state(mconfig, type, tag)
	{ }

	DECLARE_READ8_MEMBER(fdc_r);
	DECLARE_WRITE8_MEMBER(fdc_w);

	void pc6601(machine_config &config);
	void pc6601_io(address_map &map);
};

class pc6001sr_state : public pc6601_state
{
public:
	pc6001sr_state(const machine_config &mconfig, device_type type, const char *tag) :
		pc6601_state(mconfig, type, tag),
		m_sr_irq_vectors(*this, "irq_vectors")
	{ }

	DECLARE_READ8_MEMBER(hw_rev_r);
	DECLARE_READ8_MEMBER(sr_bank_rn_r);
	DECLARE_WRITE8_MEMBER(sr_bank_rn_w);
	DECLARE_READ8_MEMBER(sr_bank_wn_r);
	DECLARE_WRITE8_MEMBER(sr_bank_wn_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram0_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram1_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram2_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram3_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram4_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram5_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram6_w);
	DECLARE_WRITE8_MEMBER(sr_work_ram7_w);
	DECLARE_WRITE8_MEMBER(sr_mode_w);
	DECLARE_WRITE8_MEMBER(sr_vram_bank_w);
	DECLARE_WRITE8_MEMBER(sr_system_latch_w);
	DECLARE_WRITE8_MEMBER(necsr_ppi8255_w);
	DECLARE_WRITE8_MEMBER(sr_bitmap_yoffs_w);
	DECLARE_WRITE8_MEMBER(sr_bitmap_xoffs_w);

	INTERRUPT_GEN_MEMBER(sr_vrtc_irq);

	uint32_t screen_update_pc6001sr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pc6001sr(machine_config &config);

	void pc6001sr_io(address_map &map);
	void pc6001sr_map(address_map &map);
protected:
	virtual void video_start() override;
	virtual void machine_reset() override;

private:
	uint8_t m_sr_bank_r[8];
	uint8_t m_sr_bank_w[8];
	uint8_t m_kludge;
	bool m_sr_text_mode;
	uint8_t m_sr_text_rows;
	uint8_t *m_gvram;
	uint8_t m_bitmap_yoffs,m_bitmap_xoffs;

	enum{
		SUB_CPU_IRQ = 0,
		JOYSTICK_IRQ,
		TIMER_IRQ,
		VOICE_IRQ,
		VRTC_IRQ,
		RS232_IRQ,
		PRINTER_IRQ,
		EXT_IRQ
	};

	required_shared_ptr<uint8_t> m_sr_irq_vectors;
};

#endif
