// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/********************************************************************************************

    PC-88VA (c) 1987 NEC

********************************************************************************************/

#ifndef MAME_NEC_PC88VA_H
#define MAME_NEC_PC88VA_H

#pragma once

//#include "pc80s31k.h"
#include "pc88va_sgp.h"

#include "bus/msx/ctrl/ctrl.h"
#include "cpu/nec/v5x.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/timer.h"
#include "machine/upd1990a.h"
#include "machine/upd765.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "formats/pc98fdi_dsk.h"
#include "formats/xdf_dsk.h"


class pc88va_state : public driver_device
{
public:
	pc88va_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_fdc(*this, "upd765")
		, m_fdd(*this, "upd765:%u", 0U)
		, m_pic2(*this, "pic8259_slave")
		, m_rtc(*this, "rtc")
		, m_mouse_port(*this, "mouseport") // labelled "マウス" (mouse) - can't use "mouse" because of core -mouse option
		, m_opna(*this, "opna")
		, m_lspeaker(*this, "lspeaker")
		, m_rspeaker(*this, "rspeaker")
		, m_palram(*this, "palram")
		, m_sysbank(*this, "sysbank")
		, m_workram(*this, "workram")
		, m_tvram(*this, "tvram")
		, m_gvram(*this, "gvram")
		, m_fb_regs(*this, "fb_regs")
		, m_kanji_rom(*this, "kanji")
		, m_sgp(*this, "sgp")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void pc88va(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);

protected:
	struct tsp_t
	{
		u16 tvram_vreg_offset = 0;
		u32 attr_offset = 0;
		u32 spr_offset = 0;
		u8 spr_mg = 0;
		bool disp_on = false;
		bool spr_on = false;
		u8 pitch = 0;
		u8 line_height = 0;
		u8 h_line_pos = 0;
		u16 blink = 0;
		u16 cur_pos_x = 0, cur_pos_y = 0;
		u8 curn = 0;
		bool curn_blink = false;
		bool spwr_define = false;
		u8 spwr_offset = 0;
	};

	struct keyb_t
	{
		u8 data = 0;
	};
	keyb_t m_keyb;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void palette_init(palette_device &palette) const;

private:

	required_device<v50_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_fdd;
//  required_device<am9517a_device> m_dmac;
//  required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<upd4990a_device> m_rtc;
	required_device<msx_general_purpose_port_device> m_mouse_port;
	required_device<ym2608_device> m_opna;
	required_device<speaker_device> m_lspeaker;
	required_device<speaker_device> m_rspeaker;
	required_shared_ptr<uint16_t> m_palram;
	required_device<address_map_bank_device> m_sysbank;
	required_shared_ptr<uint16_t> m_workram;
	required_shared_ptr<uint16_t> m_tvram;
	required_shared_ptr<uint16_t> m_gvram;
	required_shared_ptr<uint16_t> m_fb_regs;
	required_region_ptr<u16> m_kanji_rom;
	required_device<pc88va_sgp_device> m_sgp;
	std::unique_ptr<uint8_t[]> m_kanjiram;

	uint16_t m_bank_reg = 0;
	uint8_t m_timer3_io_reg = 0;
	emu_timer *m_t3_mouse_timer = nullptr;
	uint8_t m_backupram_wp = 0;
	bool m_rstmd = false;

	// FDC
	emu_timer *m_fdc_timer = nullptr;
	emu_timer *m_motor_start_timer[2]{};

	uint8_t m_fdc_mode = 0;
	uint8_t m_fdc_ctrl_2 = 0;
	bool m_xtmask = false;
	TIMER_CALLBACK_MEMBER(t3_mouse_callback);
	TIMER_CALLBACK_MEMBER(pc88va_fdc_timer);
	TIMER_CALLBACK_MEMBER(pc88va_fdc_motor_start_0);
	TIMER_CALLBACK_MEMBER(pc88va_fdc_motor_start_1);
	void tc_w(int state);

	void fdc_irq(int state);
	static void floppy_formats(format_registration &fr);
	void pc88va_fdc_update_ready(floppy_image_device *, int);
	uint8_t fake_subfdc_r();
	uint8_t pc88va_fdc_r(offs_t offset);
	void pc88va_fdc_w(offs_t offset, uint8_t data);

	uint16_t bios_bank_r();
	void bios_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t rom_bank_r();
	uint8_t key_r(offs_t offset);
	void backupram_wp_1_w(uint16_t data);
	void backupram_wp_0_w(uint16_t data);
	uint8_t kanji_ram_r(offs_t offset);
	void kanji_ram_w(offs_t offset, uint8_t data);

	uint8_t hdd_status_r();

	uint16_t sysop_r();
	void timer3_ctrl_reg_w(uint8_t data);
	uint8_t backupram_dsw_r(offs_t offset);
	void sys_port1_w(uint8_t data);
	u8 sys_port5_r();
	void sys_port5_w(u8 data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(vrtc_irq);

	uint8_t r232_ctrl_porta_r();
	uint8_t r232_ctrl_portb_r();
	uint8_t r232_ctrl_portc_r();
	void r232_ctrl_porta_w(uint8_t data);
	void r232_ctrl_portb_w(uint8_t data);
	void r232_ctrl_portc_w(uint8_t data);
	uint8_t get_slave_ack(offs_t offset);

	uint16_t m_video_pri_reg[2]{};

	u16 m_screen_ctrl_reg = 0;
	bool m_dm = false;
	bool m_ymmd = false;
	u16 m_gfx_ctrl_reg = 0;

	u16 m_color_mode = 0;
	u8 m_pltm, m_pltp = 0;

	u16 m_text_transpen = 0;
	bool m_td = false;
	bitmap_rgb32 m_graphic_bitmap[2];

	u16 screen_ctrl_r();
	void screen_ctrl_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 gfx_ctrl_r();
	void gfx_ctrl_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void video_pri_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void color_mode_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void text_transpen_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void text_control_1_w(u8 data);

	u8 m_kanji_cg_line = 0;
	u8 m_kanji_cg_jis[2]{};
	u8 m_kanji_cg_lr = 0;

	u8 kanji_cg_r();
	void kanji_cg_raster_w(u8 data);
	void kanji_cg_address_w(offs_t offset, u8 data);

	void palette_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	u8 get_layer_pal_bank(u8 which);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_graphic_layer(bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 which);

	void draw_indexed_gfx_1bpp(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 fb_start_offset, u8 pal_base);
	void draw_indexed_gfx_4bpp(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 fb_start_offset, u32 display_start_offset, u8 pal_base, u16 fb_width, u16 fb_height);
	void draw_direct_gfx_8bpp(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 fb_start_offset, u16 fb_width, u16 fb_height);
	void draw_direct_gfx_rgb565(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 fb_start_offset, u16 fb_width, u16 fb_height);

	void draw_packed_gfx_4bpp(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 fb_start_offset, u32 display_start_offset, u8 pal_base, u16 fb_width, u16 fb_height);
	void draw_packed_gfx_5bpp(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 fb_start_offset, u32 display_start_offset, u8 pal_base, u16 fb_width, u16 fb_height);

	uint32_t calc_kanji_rom_addr(uint8_t jis1,uint8_t jis2,int x,int y);
	void draw_text(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// IDP
	tsp_t m_tsp;

	uint8_t m_cmd = 0;
	uint8_t m_buf_size = 0;
	uint8_t m_buf_index = 0;
	uint8_t m_buf_ram[16]{};
	u16 m_vrtc_irq_line = 432;

	uint8_t idp_status_r();
	void idp_command_w(uint8_t data);
	void idp_param_w(uint8_t data);

	void tsp_sprite_enable(u32 sprite_number, bool sprite_enable, bool blink_enable);
	void execute_sync_cmd();
	void execute_dspon_cmd();
	void execute_dspdef_cmd();
	void execute_curdef_cmd();
	void execute_actscr_cmd();
	void execute_curs_cmd();
	void execute_emul_cmd();
	void execute_spron_cmd();
	void execute_sprsw_cmd();
	void execute_spwr_cmd(u8 data);

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void sysbank_map(address_map &map) ATTR_COLD;
	void opna_map(address_map &map) ATTR_COLD;

	void sgp_map(address_map &map) ATTR_COLD;

protected:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

// TODO: stuff backported from PC8801 as QoL that should really be common
private:
	uint8_t misc_ctrl_r();
	void misc_ctrl_w(uint8_t data);
	uint8_t port40_r();
	void port40_w(offs_t offset, u8 data);
	void rtc_w(offs_t offset, u8 data);
	u8 opn_porta_r();
	u8 opn_portb_r();
	void opn_portb_w(u8 data);

	u8 m_device_ctrl_data = 0;
	u8 m_misc_ctrl = 0x80;
	bool m_sound_irq_enable = false;
	bool m_sound_irq_pending = false;
	void int4_irq_w(int state);
};


#endif // MAME_NEC_PC88VA_H
