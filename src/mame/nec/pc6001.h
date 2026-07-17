// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_NEC_PC6001_H
#define MAME_NEC_PC6001_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/msx/ctrl/ctrl.h"
#include "bus/nec_fdd/pc80s31k.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "imagedev/floppy.h"
#include "machine/74157.h"
#include "machine/bankdev.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "sound/ay8910.h"
#include "sound/upd7752.h"
#include "sound/ymopn.h"
#include "video/mc6847.h"

#include "pc6001_kbd.h"

#include "emupal.h"
#include "speaker.h"
#include "screen.h"

#include "formats/dsk_dsk.h"
#include "formats/msx_dsk.h"
//#include "formats/p6001_cas.h"


class pc6001_state : public driver_device
{
public:
	pc6001_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_ppi(*this, "ppi8255")
		, m_screen(*this, "screen")
		, m_joy(*this, "joy%u", 1U)
		, m_joymux(*this, "joymux")
		, m_cassette(*this, "cassette")
		, m_cart(*this, "cartslot")
		, m_ay(*this, "aysnd")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_region_maincpu(*this, "maincpu")
		, m_region_gfx1(*this, "gfx1")
		, m_io_mode4_dsw(*this, "MODE4_DSW")
		, m_kbd(*this, "kbd")
		, m_cart_bank(*this, "cart_bank")
		, m_palette(*this, "palette")
		, m_cas_maxsize(0)
	{ }

	void system_latch_w(uint8_t data);
	uint8_t nec_ppi8255_r(offs_t offset);
	void nec_ppi8255_w(offs_t offset, uint8_t data);

	void palette_init(palette_device &palette) const;

	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

//  INTERRUPT_GEN_MEMBER(vrtc_irq);
	TIMER_CALLBACK_MEMBER(timer_irq_cb);
	TIMER_CALLBACK_MEMBER(cassette_data_cb);

	uint8_t ppi_porta_r();
	void sub_cmd_w(uint8_t data);
	uint8_t ppi_portb_r();
	void ppi_portb_w(uint8_t data);
	void ppi_portc_w(uint8_t data);
	uint8_t ppi_portc_r();

	uint8_t joystick_r();
	uint8_t joystick_out_r();
	void joystick_out_w(uint8_t data);

	uint8_t portc0_r();

	void pc6001(machine_config &config);
protected:
	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<i8255_device> m_ppi;
	required_device<screen_device> m_screen;
	required_device_array<msx_general_purpose_port_device, 2> m_joy;
	required_device<ls157_x2_device> m_joymux;
	optional_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_cart;
	optional_device<ay8910_device> m_ay;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	optional_memory_region m_region_maincpu;
	required_memory_region m_region_gfx1;
	required_ioport m_io_mode4_dsw;
	required_device<pc6001_kbd_device> m_kbd;
	optional_device<address_map_bank_device> m_cart_bank;
	required_device<palette_device> m_palette;

	uint8_t m_timer_irq_vector = 0;
	uint16_t m_timer_hz_div = 0;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void irq_reset(u8 timer_default_setting);

	virtual void video_start() override ATTR_COLD;

	void pc6001_map(address_map &map) ATTR_COLD;
	void pc6001_io(address_map &map) ATTR_COLD;

	// i/o functions
	uint8_t check_joy_press();
	uint8_t check_keyboard_press();
	inline void cassette_motor_control(bool new_state);
	inline void ppi_control_hack_w(uint8_t data);
	inline void set_timer_divider();
	void write_centronics_busy(int state);

	// snapshot handling
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);
	emu_timer *m_cassette_timer = nullptr;

	uint8_t m_cas_switch;
	bool m_cas_motor;
	uint32_t m_cas_offset;
	uint32_t m_cas_maxsize;
	int m_cas_baud_select;
	uint8_t m_cas_data[0x18000];

	// video functions
	void draw_gfx_mode4(bitmap_ind16 &bitmap,const rectangle &cliprect,int attr);
	void draw_bitmap_2bpp(bitmap_ind16 &bitmap,const rectangle &cliprect, int attr);
	void draw_tile_3bpp(bitmap_ind16 &bitmap,const rectangle &cliprect,int x,int y,int tile,int attr);
	void draw_tile_text(bitmap_ind16 &bitmap,const rectangle &cliprect,int x,int y,int tile,int attr,int has_mc6847);
	int get_border_pen(u8 attr, int has_mc6847);
	void pc6001_screen_draw(bitmap_ind16 &bitmap,const rectangle &cliprect, int has_mc6847);

	virtual rectangle get_screen_display_area();
	virtual bool screen_blanked();
	TIMER_CALLBACK_MEMBER(video_sync_cb);
	emu_timer *m_timer_irq_timer = nullptr;
	emu_timer *m_video_sync_timer = nullptr;
	uint8_t *m_video_base = nullptr;
//  std::unique_ptr<uint8_t[]> m_video_ram;

	uint8_t m_sys_latch = 0;
	uint8_t m_bank_opt = 0;
	bool m_timer_enable = false;
	bool m_timer_irq_mask = false;
	uint8_t m_port_c_8255 = 0;
	uint8_t m_cur_keycode = 0;
	uint8_t m_centronics_busy = 0;

	void cart_map(address_map &map);

private:
	uint8_t m_joystick_out = 0xff;

// IRQ model
protected:
	// vanilla PC-6001 just maps sub CPU and Timer IRQs, mapping is otherwise confirmed by $b8-$bf vector setups in SR machines
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

	u8 timer_ack();
	u8 joystick_ack();
	u8 sub_ack();
	virtual u8 vrtc_ack();
	void set_irq_level(int which);
	void set_subcpu_irq_vector(u8 vector_num);
	IRQ_CALLBACK_MEMBER(irq_callback);

	virtual u8 get_timer_base_divider();

private:
	u8 m_irq_pending = 0;
	u8 m_sub_vector = 0;
	bool m_crtkill;
};

class pc6001mk2_state : public pc6001_state
{
public:
	pc6001mk2_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc6001_state(mconfig, type, tag)
		, m_mk2_bank(*this, "mk2_bank_%u", 0U)
		, m_gfx_view(*this, "gfx_view")
		, m_basic_rom(*this, "basic_rom")
		, m_tv_rom(*this, "tv_rom")
		, m_voice_rom(*this, "voice_rom")
		, m_kanji_rom(*this, "kanji_rom")
	{ }

	uint8_t mk2_bank_r0_r();
	uint8_t mk2_bank_r1_r();
	uint8_t mk2_bank_w0_r();
	void mk2_bank_r0_w(uint8_t data);
	void mk2_bank_r1_w(uint8_t data);
	void mk2_bank_w0_w(uint8_t data);
	void mk2_opt_bank_w(uint8_t data);
	void mk2_work_ram0_w(offs_t offset, uint8_t data);
	void mk2_work_ram1_w(offs_t offset, uint8_t data);
	void mk2_work_ram2_w(offs_t offset, uint8_t data);
	void mk2_work_ram3_w(offs_t offset, uint8_t data);
	void necmk2_ppi8255_w(offs_t offset, uint8_t data);
	void mk2_system_latch_w(uint8_t data);
	void mk2_vram_bank_w(uint8_t data);
	void mk2_col_bank_w(uint8_t data);
	void mk2_0xf3_w(uint8_t data);
	void mk2_timer_adj_w(uint8_t data);
	void mk2_timer_irqv_w(uint8_t data);

	void mk2_palette_init(palette_device &palette) const;
	void pc6001mk2(machine_config &config);

	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;

protected:
	void pc6001mk2_map(address_map &map) ATTR_COLD;
	void pc6001mk2_io(address_map &map) ATTR_COLD;

	optional_device_array<address_map_bank_device, 4> m_mk2_bank;
	memory_view m_gfx_view;

	required_memory_region m_basic_rom;
	required_memory_region m_tv_rom;
	required_memory_region m_voice_rom;
	required_memory_region m_kanji_rom;

	virtual rectangle get_screen_display_area() override;
	virtual void refresh_crtc_params();

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	virtual u8 vrtc_ack() override;

	template <unsigned TV_BASE> u8 tv_kanji_r(offs_t offset);
	template <unsigned VOICE_BASE> u8 voice_kanji_r(offs_t offset);
	template <unsigned CART_BASE> u8 cart_mk2_r(offs_t offset);

	void mk2_tv_map(address_map &map);
	template <unsigned BASIC_BASE, unsigned WORK_BASE> void mk2_voice_map(address_map &map);

	std::vector<u8> m_mk2_exram;

private:
	uint8_t m_bank_r0 = 0;
	uint8_t m_bank_r1 = 0;
	uint8_t m_bank_w = 0;
	uint8_t m_ex_vram_bank = 0;
	uint8_t m_exgfx_text_mode = 0;
	uint32_t m_cgrom_bank_addr = 0;
	uint8_t m_exgfx_bitmap_mode = 0;
	uint8_t m_exgfx_2bpp_mode = 0;
	uint8_t m_bgcol_bank = 0;

	void vram_bank_change(uint8_t vram_bank);
};

class pc6601_state : public pc6001mk2_state
{
public:
	pc6601_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc6001mk2_state(mconfig, type, tag)
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_pc80s31(*this, "pc80s31")
		, m_fdc_intf_view(*this, "fdc_intf")
	{ }

	void pc6601(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	void pc6601_io(address_map &map) ATTR_COLD;

	void pc6601_fdc_io(address_map &map) ATTR_COLD;
	void pc6601_fdc_config(machine_config &config);
	static void floppy_formats(format_registration &fr);

	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<pc80s31_device> m_pc80s31;
	memory_view m_fdc_intf_view;

	u8 fdc_mon_r();
	void fdc_mon_w(u8 data);
	void fdc_sel_w(u8 data);
};

class pc6001mk2sr_state : public pc6601_state
{
public:
	pc6001mk2sr_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc6601_state(mconfig, type, tag)
		, m_sr_bank(*this, "sr_bank_%u", 1U)
		, m_sr_irq_vectors(*this, "irq_vectors")
		, m_mk2_view(*this, "mk2_view")
		, m_mk2_io_view(*this, "mk2_io_view")
//      , m_gvram_view(*this, "gvram_view")
		, m_sr_scrollx(*this, "sr_scrollx")
		, m_sr_scrolly(*this, "sr_scrolly")
		, m_ym(*this, "ymsnd")
	{ }

	void pc6001mk2sr(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void pc6001mk2sr_map(address_map &map) ATTR_COLD;
	void sr_banked_map(address_map &map) ATTR_COLD;
	void pc6001mk2sr_io(address_map &map) ATTR_COLD;

	virtual u8 vrtc_ack() override;
	virtual u8 get_timer_base_divider() override;
	virtual rectangle get_screen_display_area() override;
	virtual bool screen_blanked() override;

private:
	required_device_array<address_map_bank_device, 16> m_sr_bank;
	required_shared_ptr<u8> m_sr_irq_vectors;
	memory_view m_mk2_view;
	memory_view m_mk2_io_view;
	required_shared_ptr<u8> m_sr_scrollx;
	required_shared_ptr<u8> m_sr_scrolly;
	required_device<ym2203_device> m_ym;

	u8 m_sr_bank_reg[16]{};
	bool m_sr_text_mode = false;
	u8 m_sr_text_rows = 0;
	std::unique_ptr<u8 []> m_gvram;
	u8 m_bitmap_yoffs = 0, m_bitmap_xoffs = 0;
	u8 m_width80 = 0;
	u8 m_sr_clut[16];
	bool m_mk2_mode;

//  memory_view m_gvram_view;

	virtual u8 hw_rev_r();
	u8 sr_bank_reg_r(offs_t offset);
	void sr_bank_reg_w(offs_t offset, u8 data);

	void sr_mode_w(u8 data);
	void sr_vram_bank_w(u8 data);
	void sr_system_latch_w(u8 data);

	virtual void refresh_crtc_params() override;
	void sr_bitmap_yoffs_w(u8 data);
	void sr_bitmap_xoffs_w(u8 data);
	void refresh_gvram_access(bool is_write);
	u8 work_ram_r(offs_t offset);
	void work_ram_w(offs_t offset, u8 data);
	u8 sr_gvram_r(offs_t offset);
	void sr_gvram_w(offs_t offset, u8 data);
	void crt_mode_w(u8 data);

	INTERRUPT_GEN_MEMBER(sr_vrtc_irq);

	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;

};

class pc6601sr_state : public pc6001mk2sr_state
{
public:
	pc6601sr_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc6001mk2sr_state(mconfig, type, tag)
	{ }

	void pc6601sr(machine_config &config);

private:
	virtual u8 hw_rev_r() override;
};

#endif // MAME_NEC_PC6001_H
